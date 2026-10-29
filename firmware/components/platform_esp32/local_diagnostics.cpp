#include "platform/local_diagnostics.hpp"
#include "driver/usb_serial_jtag.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include <cstdio>
namespace platform {
LocalDiagnostics::LocalDiagnostics(): task_(nullptr),installed_(false),archive_(nullptr),cursor_(0) {}
void LocalDiagnostics::publish(const DiagnosticSnapshot& s) {
    portENTER_CRITICAL(&lock_);
    auto latest=policy_.copy();
    latest.startup=s.startup;latest.profile_fingerprint=s.profile_fingerprint;
    latest.pwm_low_api_applied=s.pwm_low_api_applied;
    policy_.publish(latest);
    portEXIT_CRITICAL(&lock_);
}
void LocalDiagnostics::publish_capture(const bridge::CaptureSnapshot& s) {
    portENTER_CRITICAL(&lock_);auto latest=policy_.copy();
    latest.capture=s;latest.has_capture=true;policy_.publish(latest);
    portEXIT_CRITICAL(&lock_);
}
void LocalDiagnostics::archive(const bridge::AccuracyTrace* t) {
    portENTER_CRITICAL(&lock_);archive_=t;cursor_=0;portEXIT_CRITICAL(&lock_);
}
bool LocalDiagnostics::transport_ready() {
    if(installed_)return true;
    if(usb_serial_jtag_is_driver_installed())return false;
    usb_serial_jtag_driver_config_t c{};
    c.tx_buffer_size=DiagnosticTransportBuffers::tx_bytes;
    c.rx_buffer_size=DiagnosticTransportBuffers::rx_bytes;
    installed_=usb_serial_jtag_driver_install(&c)==ESP_OK;return installed_;
}
bool LocalDiagnostics::worker_ready() {
    if(task_)return true;
    if(!installed_)return false;
    task_=xTaskCreateStatic(run,"diagnostics",sizeof(task_stack_),this,1,task_stack_,&task_control_);
    return task_!=nullptr;
}
void LocalDiagnostics::run(void* p) {
    auto& self=*static_cast<LocalDiagnostics*>(p);
    for(;;){self.step(uint64_t(esp_timer_get_time()));vTaskDelay(pdMS_TO_TICKS(10));}
}
void LocalDiagnostics::step(uint64_t now) {
    bool drained=usb_serial_jtag_wait_tx_done(0)==ESP_OK;
    DiagnosticSnapshot s{};RcDiagnosticPolicy formatting;const bridge::AccuracyTrace* trace;
    portENTER_CRITICAL(&lock_);trace=archive_;
    bool ready=policy_.slot(now,drained);formatting=policy_;s=formatting.copy();portEXIT_CRITICAL(&lock_);
    if(!ready)return;
    char buffer[385];int n=0;
    if(trace) {
        const auto& h=trace->header();
        if(cursor_==0)n=std::snprintf(buffer,sizeof(buffer),"\nrc03-trace H 1 %u %u %llu %llu 0 %u %u %u %u %llu %llu %llu\n",
            unsigned(h.count),unsigned(h.failed),(unsigned long long)h.started_us,(unsigned long long)h.ended_us,
            unsigned(h.base_drops),unsigned(h.end_drops),unsigned(h.base_faults),unsigned(h.end_faults),
            (unsigned long long)h.max_rearm_us,(unsigned long long)h.max_processing_us,
            (unsigned long long)h.max_fault_lateness_us);
        else if(cursor_<=h.count) {
            const auto& x=trace->record(cursor_-1);
            n=std::snprintf(buffer,sizeof(buffer),"\nrc03-trace R %u %u %u %llu %llu %u %u %u %u %llu\n",
                cursor_-1,unsigned(x.seq),unsigned(x.epoch),(unsigned long long)x.start_us,
                (unsigned long long)x.end_us,unsigned(x.width_us),unsigned(x.period_us),unsigned(x.flags),unsigned(x.fault_bits),(unsigned long long)x.accepted_at_us);
        } else n=std::snprintf(buffer,sizeof(buffer),"\nrc03-trace E %u\n",unsigned(h.count));
    } else {
#ifdef CONFIG_BRIDGE_RC03_DIAGNOSTIC_STRESS
        const bool padded=true;
#else
        const bool padded=false;
#endif
        n=formatting.format(s,buffer,sizeof(buffer),padded);
    }
    size_t sent=0;
    if(n>0 && n<=384) {
        int w=usb_serial_jtag_write_bytes(buffer,size_t(n),0);if(w>0)sent=size_t(w);
    }
    portENTER_CRITICAL(&lock_);
    policy_.written(sent,n>0?size_t(n):1);
    if(n>0 && sent==size_t(n)) {
        if(trace) { if(++cursor_>trace->header().count+1)cursor_=0; }
        else policy_.ack(s.sequence);
    }
    portEXIT_CRITICAL(&lock_);
}
}
