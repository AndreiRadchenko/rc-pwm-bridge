#include "platform/pump_output.hpp"
#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#endif
namespace platform {
using namespace bridge;
EspPumpOutput::EspPumpOutput(PumpSdk& s,bool b,OutputEvidence* e):sdk_(s),evidence_(e),bench_(b){}
void EspPumpOutput::fail(uint8_t reason) {
 reasons_|=reason;terminal_=true;
 if(!fault_set_) {fault_set_=true;fault_at_=sdk_.now_us();}
}
int32_t EspPumpOutput::call(PumpOperation op,uint32_t value,bool nonzero) {
 if(nonzero&&(terminal_||sdk_.faults())){fail(PumpCaptureFault);return INT32_MIN;}
 const uint64_t before=sdk_.now_us();
 const int32_t status=sdk_.invoke(op,value,config_);
 const uint64_t after=sdk_.now_us();
 if(after<before||before<last_time_)fail(PumpClockError);
 last_time_=after;
 if(status){fail(PumpApplyApiError);if(op!=PumpOperation::Cleanup)applied_=PumpApplied::Unknown;}
 if(op==PumpOperation::GpioLow)gpio_low_latched_=status==0;
 if(!status) {
  if(op==PumpOperation::Stop)applied_=value?PumpApplied::High:PumpApplied::Low;
  if(op==PumpOperation::Update)applied_=PumpApplied::Pwm;
  if(op==PumpOperation::GpioDirection)applied_=gpio_low_latched_?PumpApplied::Low:PumpApplied::Unknown;
 }
 if(nonzero&&sdk_.faults())fail(PumpCaptureFault);
 const uint64_t start=fault_set_?fault_at_:deadline_;
 if((fault_set_&&after>=start&&after-start>1000)||(!fault_set_&&has_generation_&&after>=deadline_&&after-deadline_>2000))fail(PumpDeadlineLate);
 if(nonzero&&terminal_)applied_=PumpApplied::Unknown;
 if(evidence_&&!evidence_->append({0,uint8_t(op),percent_,uint8_t(applied_),reasons_,status,before,after,start}))fail(PumpEvidenceOverflow);
 return status;
}
bool EspPumpOutput::low() {
 bool ok=false;const bool fallback_needed=configured_||terminal_;
 if(configured_)ok=call(PumpOperation::Stop,0)==0;
 if(!ok){const int32_t a=call(PumpOperation::GpioLow,0);const int32_t d=call(PumpOperation::GpioDirection,0);ok=a==0&&d==0;if(evidence_&&fallback_needed){evidence_->fallback(PumpOperation::GpioLow,a);evidence_->fallback(PumpOperation::GpioDirection,d);}}
 if(!ok){applied_=PumpApplied::Unknown;fail(PumpLowUnconfirmed);}else applied_=PumpApplied::Low;
 return ok;
}
bool EspPumpOutput::initialize() {
 if(initialized_)return !terminal_;
 initialized_=true;percent_=0;
 if(!low()){fail(PumpInitFailure);return false;}
 if(terminal_){fail(PumpInitFailure);return false;}
 if(bench_) {
  configured_=true;
  if(call(PumpOperation::Init,0)!=0){fail(PumpInitFailure);low();return false;}
  channel_=true;
  if(call(PumpOperation::Init,1)!=0){fail(PumpInitFailure);low();return false;}
 }
 if(terminal_){low();return false;}
 return true;
}
OutputApplyResult EspPumpOutput::apply_low() {
 if(!initialized_)return {initialize()};
 return {applied_==PumpApplied::Low||low()};
}
bool EspPumpOutput::apply(const OutputRequest& q) {
 if(!initialized_&&!initialize())return false;
 if(q.reasons){if(!fault_set_){fault_set_=true;fault_at_=q.deadline_us;}fail(q.reasons);}
 if(sdk_.faults())fail(PumpCaptureFault);
 if(q.percent>100||(!bench_&&q.permission&&q.percent))fail(PumpInvalidRequest);
 if(has_generation_&&q.generation<generation_&&q.permission&&q.percent)fail(PumpInvalidRequest);
 if(has_generation_&&q.generation==generation_&&q.permission&&q.percent&&q.percent!=percent_)fail(PumpInvalidRequest);
 const uint8_t percent=(!terminal_&&q.permission)?q.percent:0;
 if(has_generation_&&q.generation==generation_&&percent==percent_&&(!terminal_||applied_==PumpApplied::Low))return !terminal_;
 if(!has_generation_||q.generation>generation_)generation_=q.generation;
 has_generation_=true;deadline_=q.deadline_us;percent_=percent;
 bool attempted_low=false;
 if(percent==0){if(applied_!=PumpApplied::Low||!has_low_request_){low();attempted_low=true;}has_low_request_=true;}
 else if(percent==100){call(PumpOperation::Stop,1,true);has_low_request_=false;}
 else {uint32_t count=0;pump_count(percent,count);if(call(PumpOperation::Set,count,true)==0&&!terminal_)call(PumpOperation::Update,0,true);has_low_request_=false;}
 if(terminal_&&!attempted_low&&applied_!=PumpApplied::Low)low();
 return !terminal_;
}
void EspPumpOutput::cleanup() {
 if(!low())return;
 if(configured_){call(PumpOperation::Cleanup,0);call(PumpOperation::Cleanup,1);}
 configured_=channel_=false;
}
#ifdef ESP_PLATFORM
uint64_t EspPumpSdk::now_us(){return uint64_t(esp_timer_get_time());}
uint32_t EspPumpSdk::faults(){return reader_?reader_(context_):0;}
int32_t EspPumpSdk::invoke(PumpOperation op,uint32_t value,const PumpHardwareConfig& c) {
 const ledc_mode_t mode=LEDC_LOW_SPEED_MODE;const ledc_channel_t channel=LEDC_CHANNEL_0;
 if(op==PumpOperation::GpioLow)return gpio_set_level(gpio_num_t(c.pin),0);
 if(op==PumpOperation::GpioDirection)return gpio_set_direction(gpio_num_t(c.pin),GPIO_MODE_OUTPUT);
 if(op==PumpOperation::Init&&value==0){ledc_timer_config_t t{};t.speed_mode=mode;t.duty_resolution=ledc_timer_bit_t(c.resolution);t.timer_num=LEDC_TIMER_0;t.freq_hz=c.frequency_hz;t.clk_cfg=LEDC_USE_APB_CLK;return ledc_timer_config(&t);}
 if(op==PumpOperation::Init){ledc_channel_config_t x{};x.gpio_num=c.pin;x.speed_mode=mode;x.channel=channel;x.intr_type=LEDC_INTR_DISABLE;x.timer_sel=LEDC_TIMER_0;x.duty=0;x.hpoint=0;x.sleep_mode=LEDC_SLEEP_MODE_NO_ALIVE_NO_PD;return ledc_channel_config(&x);}
 if(op==PumpOperation::Set)return ledc_set_duty_with_hpoint(mode,channel,value,0);
 if(op==PumpOperation::Update)return ledc_update_duty(mode,channel);
 if(op==PumpOperation::Stop)return ledc_stop(mode,channel,value);
 if(op==PumpOperation::Cleanup&&value==0)return ledc_timer_pause(mode,LEDC_TIMER_0);
 if(op==PumpOperation::Cleanup){ledc_timer_config_t t{};t.speed_mode=mode;t.timer_num=LEDC_TIMER_0;t.deconfigure=true;return ledc_timer_config(&t);}
 return ESP_ERR_INVALID_ARG;
}
#endif
}

#include <cstdio>
namespace platform {
static int archive_tail(char* b,size_t cap,int n,const DiagnosticSnapshot& s) {
 if(n<=0||size_t(n)>=cap)return 0;
 int tail=std::snprintf(b+n-1,cap-size_t(n)+1," cf=%x of=%x pf=%x\n",unsigned(s.capture.fault_bits),unsigned(s.output.reasons),unsigned(s.window_invalid));
 if(tail<=0||size_t(n-1+tail)>=cap||n-1+tail>384)return 0;
 return n-1+tail;
}
int format_output_archive(const bridge::OutputEvidence& e,unsigned cursor,const DiagnosticSnapshot& s,char* b,size_t cap) {
 const auto& h=e.header();if(!h.frozen||unsigned(h.count)>64)return 0;int n=0;
 if(cursor==0)n=std::snprintf(b,cap,"\nrc04-out H 1 %llx %llx %x %llx %x %x %x %x %x %x %x %x %x %x\n",(unsigned long long)h.fingerprint,(unsigned long long)h.boot,unsigned(h.scenario),(unsigned long long)h.t0_us,unsigned(h.count),unsigned(h.first_seq),unsigned(h.last_seq),unsigned(h.frozen),unsigned(h.overflow),unsigned(h.first_operation),unsigned(uint32_t(h.first_error)),unsigned(h.fallback_attempted),unsigned(h.fallback_operation),unsigned(uint32_t(h.fallback_error)));
 else if(cursor<=unsigned(h.count)) {const auto& r=e.record(cursor-1);n=std::snprintf(b,cap,"\nrc04-out R 1 %llx %llx %x %x %x %x %x %x %x %llx %llx %llx\n",(unsigned long long)h.fingerprint,(unsigned long long)h.boot,cursor-1,unsigned(r.seq),unsigned(r.kind),unsigned(r.duty),unsigned(r.applied),unsigned(r.reasons),unsigned(uint32_t(r.sdk_status)),(unsigned long long)r.before_us,(unsigned long long)r.after_us,(unsigned long long)r.deadline_us);}
 else if(cursor==unsigned(h.count)+1)n=std::snprintf(b,cap,"\nrc04-out E 1 %llx %llx %x %llx\n",(unsigned long long)h.fingerprint,(unsigned long long)h.boot,unsigned(h.count),(unsigned long long)e.digest());
 return archive_tail(b,cap,n,s);
}
int format_window_archive(const bridge::CaptureWindowEvidence& w,const bridge::OutputHeader& id,unsigned cursor,const DiagnosticSnapshot& s,char* b,size_t cap) {
 const auto& h=w.header();if(!w.frozen())return 0;int n=0;
 if(cursor==0)n=std::snprintf(b,cap,"\nrc04-win H 1 %llx %llx %llx %llx %llx %llx %llx %x %x %x %x %x %x %x 3\n",(unsigned long long)id.fingerprint,(unsigned long long)id.boot,(unsigned long long)h.start_us,(unsigned long long)h.end_us,(unsigned long long)h.start_checkpoint_us,(unsigned long long)h.end_checkpoint_us,(unsigned long long)h.finalized_us,unsigned(h.watermark_seq),unsigned(h.watermark_epoch),unsigned(h.flags),unsigned(h.has_start_checkpoint),unsigned(h.has_end_checkpoint),unsigned(h.has_samples),unsigned(h.has_fault_delivery));
 else if(cursor==1)n=std::snprintf(b,cap,"\nrc04-win B 1 %llx %llx %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",(unsigned long long)id.fingerprint,(unsigned long long)id.boot,unsigned(h.baseline[0]),unsigned(h.baseline[1]),unsigned(h.baseline[2]),unsigned(h.baseline[3]),unsigned(h.baseline[4]),unsigned(h.baseline[5]),unsigned(h.baseline[6]),unsigned(h.ending[0]),unsigned(h.ending[1]),unsigned(h.ending[2]),unsigned(h.ending[3]),unsigned(h.ending[4]),unsigned(h.ending[5]),unsigned(h.ending[6]));
 else if(cursor==2)n=std::snprintf(b,cap,"\nrc04-win W 1 %llx %llx %x %x %x %x %x %x %x %x %x %x %x %x %llx %llx\n",(unsigned long long)id.fingerprint,(unsigned long long)id.boot,unsigned(h.counts[0]),unsigned(h.counts[1]),unsigned(h.counts[2]),unsigned(h.counts[3]),unsigned(h.counts[4]),unsigned(h.counts[5]),unsigned(h.counts[6]),unsigned(h.counts[7]),unsigned(h.first_seq),unsigned(h.last_seq),unsigned(h.first_epoch),unsigned(h.last_epoch),(unsigned long long)h.first_end_us,(unsigned long long)h.last_end_us);
 else if(cursor==3)n=std::snprintf(b,cap,"\nrc04-win T 1 %llx %llx %llx %llx %llx\n",(unsigned long long)id.fingerprint,(unsigned long long)id.boot,(unsigned long long)h.max_rearm_us,(unsigned long long)h.max_processing_us,(unsigned long long)h.max_fault_delivery_us);
 else if(cursor==4)n=std::snprintf(b,cap,"\nrc04-win E 1 %llx %llx 3 %llx\n",(unsigned long long)id.fingerprint,(unsigned long long)id.boot,(unsigned long long)w.digest());
 return archive_tail(b,cap,n,s);
}
}
