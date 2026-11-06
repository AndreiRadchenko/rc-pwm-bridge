#include "platform/rc_capture.hpp"
#include "board_profile.hpp"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp_timer.h"
#include "soc/gpio_struct.h"
namespace platform {
static constexpr gpio_num_t input =
    static_cast<gpio_num_t>(BRIDGE_PROFILE_RC_PIN);
RcCapture::RcCapture(LocalDiagnostics &d, EspPumpOutput& out, EspPumpSdk& sdk, bridge::BenchConfig config, bridge::OutputEvidence* evidence, bridge::CaptureWindowEvidence* window) : diagnostics_(d), output_(out), output_policy_(config), output_evidence_(evidence), window_(window) {
  sdk.observer(output_faults,this);
  edges_ = xQueueCreateStatic(64, sizeof(bridge::EdgeEvent), edge_storage_,
                              &edge_control_);
  batches_ = xQueueCreateStatic(8, sizeof(bridge::RxBatch), batch_storage_,
                                &batch_control_);
  receive_.signal_range_min_ns = 0;
  receive_.signal_range_max_ns = 5000000;
}
bool RcCapture::acquire(bridge::CaptureResource resource) {
  using bridge::CaptureResource;
  if (resource == CaptureResource::Rmt) {
    rmt_rx_channel_config_t c{};
    c.gpio_num = input;
    c.clk_src = RMT_CLK_SRC_APB;
    c.resolution_hz = 1000000;
    c.mem_block_symbols = 48;
    c.intr_priority = 3;
    if (rmt_new_rx_channel(&c, &channel_) != ESP_OK)
      return false;
    rmt_rx_event_callbacks_t callbacks{};
    callbacks.on_recv_done = rx_done;
    // RMT enables an internal pull-up; the external RC divider supplies bias.
    if (gpio_set_pull_mode(input, GPIO_FLOATING) != ESP_OK ||
        rmt_rx_register_event_callbacks(channel_, &callbacks, this) != ESP_OK ||
        rmt_enable(channel_) != ESP_OK) {
      rmt_del_channel(channel_);
      channel_ = nullptr;
      return false;
    }
    enabled_ = true;
    return true;
  }
  if (resource == CaptureResource::Gpio) {
    esp_err_t e =
        gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3);
    // An existing service exposes neither its IRQ priority nor IRAM allocation.
    // Leave foreign ownership untouched and fail closed instead of reusing it.
    if (e != ESP_OK)
      return false;
    gpio_service_owned_ = e == ESP_OK;
    if (gpio_set_intr_type(input, GPIO_INTR_ANYEDGE) != ESP_OK ||
        gpio_isr_handler_add(input, gpio_edge, this) != ESP_OK) {
      if (gpio_service_owned_)
        gpio_uninstall_isr_service();
      gpio_service_owned_ = false;
      return false;
    }
    gpio_added_ = true;
    gpio_intr_disable(input);
    return true;
  }
  if (resource == CaptureResource::Timer) {
    esp_timer_create_args_t t{};
    t.callback = tick;
    t.arg = this;
    t.name = "rc-monitor";
    t.dispatch_method = ESP_TIMER_TASK;
    t.skip_unhandled_events = true;
    return esp_timer_create(&t, &timer_) == ESP_OK;
  }
  task_ = xTaskCreateStatic(run, "rc-control", sizeof(stack_), this, 10, stack_,
                            &task_control_);
  return task_ != nullptr;
}
void RcCapture::release(bridge::CaptureResource r) {
  using bridge::CaptureResource;
  if (r == CaptureResource::Worker) {
    portENTER_CRITICAL(&lock_);
    auto task = task_;
    task_ = nullptr;
    portEXIT_CRITICAL(&lock_);
    if (task)
      vTaskDelete(task);
  }
  if (r == CaptureResource::Timer && timer_) {
    if(esp_timer_stop(timer_)!=ESP_OK)policy_.fault(bridge::RxError,uint64_t(esp_timer_get_time()));
    esp_timer_delete(timer_);
    timer_ = nullptr;
  }
  if (r == CaptureResource::Gpio) {
    if (gpio_added_)
      gpio_isr_handler_remove(input);
    if (gpio_service_owned_)
      gpio_uninstall_isr_service();
    gpio_added_ = false;
    gpio_service_owned_ = false;
  }
  if (r == CaptureResource::Rmt && channel_) {
    if (enabled_)
      rmt_disable(channel_);
    rmt_del_channel(channel_);
    channel_ = nullptr;
    enabled_ = false;
  }
}
bool RcCapture::start(uint64_t now) {
  if (running_)
    return true;
  if (now < retry_at_)
    return false;
  if (!initialized_) {
    policy_.start(now, false);
    initialized_ = true;
    started_at_ = now;
  }
  if (!edges_ || !batches_ || !resources_.setup(*this, now)) {
    if (now >= retry_at_) {
      policy_.fault(bridge::RxError, now);
      retry_at_ = now + 100000;
      diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),window_&&window_->postfreeze_invalid());
    }
    return false;
  }
  if (esp_timer_start_periodic(timer_, 1000) != ESP_OK) {
    resources_.cleanup(*this);
    policy_.fault(bridge::RxError, now);
    retry_at_ = now + 100000;
    diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),window_&&window_->postfreeze_invalid());
    return false;
  }
  portENTER_CRITICAL(&lock_);
  bool high = gpio_get_level(input);
  line_ = {now, now, line_.seq, high, true, true};
  recovery_requested_ = false;
  policy_.recovered(now, high, line_.seq);
  running_ = true;
  gpio_intr_enable(input);
  bool armed = high || arm_locked(false);
  if (!armed)
    pending_fault_ |= bridge::RxError;
  portEXIT_CRITICAL(&lock_);
  xTaskNotifyGive(task_);
  return true;
}
bool RcCapture::pending_edge() const {
  // Same volatile register read as pinned C3 gpio_ll_get_intr_status, without
  // GNU-only HAL helpers.
  uint32_t status = GPIO.pcpu_int.intr;
  return status & (1u << unsigned(input));
}
bool RcCapture::arm_locked(bool) {
  if (rx_active_ || recovery_requested_ || stopped_)
    return false;
  if (gpio_get_level(input))
    return false;
  if (pending_edge()) {
    pending_fault_ |= bridge::ShapeError;
    return false;
  }
  const uint32_t before = line_.seq;
  ++epoch_;
  arm_seq_ = before;
  armed_at_ = uint64_t(esp_timer_get_time());
  if (rmt_receive(channel_, scratch_, sizeof(scratch_), &receive_) != ESP_OK) {
    pending_fault_ |= bridge::RxError;
    recovery_requested_ = true;
    return false;
  }
  rx_active_ = true;
  if (gpio_get_level(input) || line_.seq != before || pending_edge()) {
    pending_fault_ |= bridge::ShapeError;
    recovery_requested_ = true;
    return false;
  }
  return true;
}
void RcCapture::gpio_edge(void *data) {
  auto &s = *static_cast<RcCapture *>(data);
  uint64_t now = uint64_t(esp_timer_get_time());
  bool high = gpio_get_level(input);
  BaseType_t wake = pdFALSE;
  portENTER_CRITICAL_ISR(&s.lock_);
  ++s.producers_;
  bridge::EdgeEvent event{now, ++s.line_.seq, s.epoch_, high};
  if (high == s.line_.high)
    s.pending_fault_ |= bridge::ShapeError;
  else {
    s.line_.high = high;
    s.line_.changed_us = now;
  }
  portEXIT_CRITICAL_ISR(&s.lock_);
  const bool lost=xQueueSendFromISR(s.edges_, &event, &wake) != pdTRUE;
  portENTER_CRITICAL_ISR(&s.lock_);
  if (lost) {
    if (s.pending_losses_ == UINT32_MAX)
      s.pending_fault_ |= bridge::Saturation;
    else
      ++s.pending_losses_;
    s.pending_fault_ |= bridge::QueueLoss;
    s.recovery_requested_ = true;
    s.line_.continuous = false;
    gpio_intr_disable(input);
  }
  --s.producers_;
  if (s.task_)
    vTaskNotifyGiveFromISR(s.task_, &wake);
  portEXIT_CRITICAL_ISR(&s.lock_);
  if (wake)
    portYIELD_FROM_ISR(wake);
}
bool RcCapture::rx_done(rmt_channel_handle_t, const rmt_rx_done_event_data_t *e,
                        void *data) {
  auto &s = *static_cast<RcCapture *>(data);
  bridge::RxBatch batch{};
  batch.completed_at_us = uint64_t(esp_timer_get_time());
  portENTER_CRITICAL_ISR(&s.lock_);
  ++s.producers_;
  batch.epoch = s.epoch_;
  batch.arm_seq = s.arm_seq_;
  batch.close_seq = s.line_.seq;
  batch.armed_at_us = s.armed_at_;
  batch.final_high = s.line_.high;
  s.rx_active_ = false;
  portEXIT_CRITICAL_ISR(&s.lock_);
  batch.symbol_count = e->num_symbols >= 48 ? 48 : uint16_t(e->num_symbols);
  unsigned count = e->num_symbols < 48 ? e->num_symbols : 48;
  for (unsigned i = 0; i < count; ++i) {
    const auto v = e->received_symbols[i];
    batch.runs[2 * i] = {uint16_t(v.duration0), bool(v.level0)};
    batch.runs[2 * i + 1] = {uint16_t(v.duration1), bool(v.level1)};
  }
  batch.run_count = count * 2;
  portENTER_CRITICAL_ISR(&s.lock_);
  if (!s.recovery_requested_ && !s.stopped_ && !s.line_.high) {
    if (s.arm_locked(true))
      batch.rearmed_at_us = uint64_t(esp_timer_get_time());
  }
  portEXIT_CRITICAL_ISR(&s.lock_);
  BaseType_t wake = pdFALSE;
  const bool lost=xQueueSendFromISR(s.batches_, &batch, &wake) != pdTRUE;
  portENTER_CRITICAL_ISR(&s.lock_);
  if (lost) {
    if (s.pending_losses_ == UINT32_MAX)
      s.pending_fault_ |= bridge::Saturation;
    else
      ++s.pending_losses_;
    s.pending_fault_ |= bridge::QueueLoss;
    s.recovery_requested_ = true;
  }
  --s.producers_;
  if (s.task_)
    vTaskNotifyGiveFromISR(s.task_, &wake);
  portEXIT_CRITICAL_ISR(&s.lock_);
  return wake == pdTRUE;
}
void RcCapture::tick(void *p) {
  auto &s = *static_cast<RcCapture *>(p);
  portENTER_CRITICAL(&s.lock_);
  if (s.task_)
    xTaskNotifyGive(s.task_);
  portEXIT_CRITICAL(&s.lock_);
}
void RcCapture::run(void *p) {
  auto &s = *static_cast<RcCapture *>(p);
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (s.running_)
      s.step();
  }
}
void RcCapture::recovery(uint64_t now) {
  portENTER_CRITICAL(&lock_);
  recovery_requested_ = true;
  gpio_intr_disable(input);
  portEXIT_CRITICAL(&lock_);
  esp_err_t off = enabled_ ? rmt_disable(channel_) : ESP_OK;
  if (off != ESP_OK) {
    retry_at_ = now + 100000;
    policy_.fault(bridge::RxError, now);
    return;
  }
  enabled_ = false;
  if (rmt_enable(channel_) != ESP_OK) {
    retry_at_ = now + 100000;
    policy_.fault(bridge::RxError, now);
    return;
  }
  enabled_ = true;
  xQueueReset(edges_);
  xQueueReset(batches_);
  portENTER_CRITICAL(&lock_);
  bool high = gpio_get_level(input);
  rx_active_ = false;
  line_ = {now, now, line_.seq, high, true, true};
  recovery_requested_ = false;
  policy_.recovered(now, high, line_.seq);
  gpio_intr_enable(input);
  if (!high)
    arm_locked(false);
  portEXIT_CRITICAL(&lock_);
}
void RcCapture::step() {
  if (stopped_)
    return;
  uint64_t now = uint64_t(esp_timer_get_time());
  uint32_t errors = 0, losses = 0;
  portENTER_CRITICAL(&lock_);
  errors = pending_fault_;
  pending_fault_ = 0;
  losses = pending_losses_;
  pending_losses_ = 0;
  portEXIT_CRITICAL(&lock_);
  if (errors)
    policy_.fault(errors, now, losses);
  output_service(uint64_t(esp_timer_get_time()));
  bridge::EdgeEvent edge;
  for (unsigned i = 0; i < 64 && xQueueReceive(edges_, &edge, 0) == pdTRUE; ++i) {
    policy_.on_edge(edge, uint64_t(esp_timer_get_time()));
    output_service(uint64_t(esp_timer_get_time()));
  }
  bridge::RxBatch batch;
  for (unsigned i = 0; i < 8 && xQueueReceive(batches_, &batch, 0) == pdTRUE;
       ++i) {
    output_service(uint64_t(esp_timer_get_time()));
    bool candidate = policy_.on_batch(batch, uint64_t(esp_timer_get_time()));
    policy_.processing_complete(batch.completed_at_us,
                                uint64_t(esp_timer_get_time()));
#ifdef CONFIG_BRIDGE_RC03_ACCURACY_TRACE
    if (candidate && (output_policy_.config().scenario!=bridge::PumpScenario::Accuracy || trace_enabled_))
      trace_.append(policy_.snapshot(), uint64_t(esp_timer_get_time()));
#else
    (void)candidate;
#endif
    if(window_&&candidate){const auto m=policy_.snapshot().measurement;
      const uint64_t completion=uint64_t(esp_timer_get_time());
      const uint64_t stop=m.end_us<=UINT64_MAX-5000?m.end_us+5000:UINT64_MAX;
      const uint64_t rearm=batch.rearmed_at_us?(batch.rearmed_at_us>=stop?batch.rearmed_at_us-stop:0):UINT64_MAX;
      window_->sample(m,rearm,completion>=batch.completed_at_us?completion-batch.completed_at_us:UINT64_MAX);
    }
    output_service(uint64_t(esp_timer_get_time()));
  }
  bridge::LineObservation line;
  portENTER_CRITICAL(&lock_);
  line = line_;
  portEXIT_CRITICAL(&lock_);
  portENTER_CRITICAL(&lock_);
  line.stable = line.seq == line_.seq;
  portEXIT_CRITICAL(&lock_);
  now = uint64_t(esp_timer_get_time());
  policy_.poll(line, now);
  output_service(uint64_t(esp_timer_get_time()));
#ifdef CONFIG_BRIDGE_RC03_ACCURACY_TRACE
  trace_.observe(policy_.snapshot());
  if(output_.reasons())trace_.fail();
  if (trace_.header().frozen && !stopped_) {
    output_policy_.freeze();
    output_service(uint64_t(esp_timer_get_time()));
    if(output_.reasons())trace_.fail();
    if(output_.applied()!=bridge::PumpApplied::Low || (output_.reasons()&bridge::PumpLowUnconfirmed)) {
      diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),window_&&window_->postfreeze_invalid());
      return;
    }
    portENTER_CRITICAL(&lock_);
    stopped_ = true;
    recovery_requested_ = true;
    gpio_intr_disable(input);
    portEXIT_CRITICAL(&lock_);
    bool stop_ok=true;
    if (enabled_) {
      if(rmt_disable(channel_)!=ESP_OK){policy_.fault(bridge::RxError,uint64_t(esp_timer_get_time()));stop_ok=false;}
      else enabled_=false;
    }
    if(esp_timer_stop(timer_)!=ESP_OK){policy_.fault(bridge::RxError,uint64_t(esp_timer_get_time()));stop_ok=false;}
    output_service(uint64_t(esp_timer_get_time()));
    if(!stop_ok){trace_.fail();if(output_evidence_)diagnostics_.output_archive(output_evidence_,nullptr);diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),false);return;}
    diagnostics_.archive(&trace_);
    if(output_evidence_)diagnostics_.output_archive(output_evidence_,nullptr);
    diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),window_&&window_->postfreeze_invalid());
    return;
  }
#endif
  auto state = policy_.snapshot();
  output_service(uint64_t(esp_timer_get_time()));
  if (state.recovery_needed && now >= retry_at_)
    recovery(now);
  else {
    portENTER_CRITICAL(&lock_);
    if (!rx_active_ && !recovery_requested_ && !line_.high && !stopped_)
      arm_locked(false);
    portEXIT_CRITICAL(&lock_);
  }
  if(window_) {
    const auto h=window_->header();
    if(now>=h.end_us+6000&&!window_->frozen()) {
      portENTER_CRITICAL(&lock_);
      const uint32_t seq=line_.seq,epoch=epoch_;
      const bool relevant_active=rx_active_&&line_.seq!=arm_seq_&&line_.changed_us<h.end_us;
      const bool barrier=producers_==0&&!pending_fault_&&!pending_edge()&&!relevant_active&&uxQueueMessagesWaiting(edges_)==0&&uxQueueMessagesWaiting(batches_)==0;
      portEXIT_CRITICAL(&lock_);
      window_->close(policy_.snapshot(),uint64_t(esp_timer_get_time()),barrier,seq,epoch);
    }
  }
  if(output_evidence_&&output_evidence_->header().frozen&&(!window_||window_->frozen()))diagnostics_.output_archive(output_evidence_,window_);
  diagnostics_.publish_control(policy_.snapshot(),output_policy_.snapshot(),window_&&window_->postfreeze_invalid());
}
uint32_t RcCapture::output_faults(void* p) {
  auto& s=*static_cast<RcCapture*>(p);
  portENTER_CRITICAL(&s.lock_);const uint32_t bits=s.pending_fault_;portEXIT_CRITICAL(&s.lock_);
  return bits|s.policy_.snapshot().fault_bits;
}
void RcCapture::output_service(uint64_t now) {
  const bool first=!output_started_;output_started_=true;
  auto q=output_policy_.tick(now,{true,startup_failed_,running_.load(),reset_reason_},policy_.snapshot().fault_bits?bridge::PumpCaptureFault:0);
  if(policy_.snapshot().fault_bits && policy_.snapshot().first_fault_at_us<=now)q.deadline_us=policy_.snapshot().first_fault_at_us;
  if(first) {
    if(output_evidence_) {output_evidence_->begin(now);output_evidence_->append({0,uint8_t(bridge::PumpOperation::SessionStart),0,uint8_t(output_.applied()),0,0,now,now,now});}
    if(window_)window_->start(now);
  }
  output_.apply(q);
  if(output_.reasons())output_policy_.fault(output_.reasons());
  output_policy_.applied(output_.applied());
  trace_enabled_=trace_enabled_||output_policy_.accuracy_collect(now);
  if(window_)window_->observe(policy_.snapshot(),now);
  auto phase=output_policy_.snapshot().phase;
  if(output_evidence_&&!output_evidence_->header().frozen&&(phase==bridge::PumpPhase::Fault||phase==bridge::PumpPhase::Complete||phase==bridge::PumpPhase::EvidenceFrozen)) {
    const auto kind=phase==bridge::PumpPhase::Fault?bridge::PumpOperation::Fault:phase==bridge::PumpPhase::Complete?bridge::PumpOperation::Complete:bridge::PumpOperation::EvidenceFrozen;
    output_evidence_->append({0,uint8_t(kind),0,uint8_t(output_.applied()),output_.reasons(),0,now,uint64_t(esp_timer_get_time()),q.deadline_us});
    output_evidence_->freeze();
  }
}
} // namespace platform
