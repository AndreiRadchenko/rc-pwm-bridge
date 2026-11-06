#include "bridge/pump_output.hpp"
#include <initializer_list>
namespace bridge {
BenchConfig BenchConfig::normal() { return {PumpScenario::Normal,0}; }
BenchConfig BenchConfig::bench(PumpScenario s,uint8_t step) { return {s,step}; }
bool BenchConfig::valid() const { return uint8_t(scenario)<=3 && (scenario==PumpScenario::Accuracy ? trace_step<=5 : trace_step==0); }
uint64_t BenchConfig::plateau_us() const { return scenario==PumpScenario::Stress?100000000ULL:10000000ULL; }
bool pump_count(uint32_t percent,uint32_t& count) { if(percent>100)return false; count=(percent*8192u+50u)/100u;return true; }
OutputPolicy::OutputPolicy(BenchConfig c):config_(c),state_{},last_now_(0),command_deadline_(0),started_(false) {
 state_.mode=c.scenario==PumpScenario::Normal?PumpMode::Normal:PumpMode::Bench;
 state_.scenario=c.scenario;state_.phase=PumpPhase::Disabled;
}
void OutputPolicy::select(PumpPhase phase,uint8_t percent,bool permission,uint64_t deadline) {
 if(state_.phase==phase&&state_.requested_percent==percent&&state_.bench_permission==permission&&state_.deadline_us==deadline&&state_.generation)return;
 if(state_.generation==UINT64_MAX) { state_.reasons|=PumpClockError;phase=PumpPhase::Fault;percent=0;permission=false; }
 else ++state_.generation;
 command_deadline_=(uint8_t(phase)>=2&&uint8_t(phase)<=8&&uint8_t(state_.phase)>=1&&uint8_t(state_.phase)<=7)?state_.deadline_us:last_now_;
 state_.phase=phase;state_.requested_percent=percent;state_.bench_permission=permission;state_.deadline_us=deadline;
}
void OutputPolicy::fault(uint8_t reason) { state_.reasons|=reason;if(state_.phase!=PumpPhase::Fault)select(PumpPhase::Fault,0,false,last_now_); }
void OutputPolicy::freeze() { if(state_.phase!=PumpPhase::Fault)select(PumpPhase::EvidenceFrozen,0,false,last_now_); }
OutputRequest OutputPolicy::request() const { return {state_.generation,command_deadline_,state_.requested_percent,state_.reasons,state_.bench_permission}; }
OutputRequest OutputPolicy::tick(uint64_t now,BootContext b,uint8_t errors) {
 if(started_&&now<last_now_) { fault(PumpClockError);return request(); }
 last_now_=now;
 if(!started_) {
  started_=true;state_.t0_us=now;
  if(now>UINT64_MAX-606017000ULL)fault(PumpClockError);
  else if(config_.scenario!=PumpScenario::Normal)select(PumpPhase::InitialLow,0,false,now+5000000);
 }
 if(!config_.valid())fault(PumpInvalidRequest);
 if(b.startup_had_failure)fault(PumpInitFailure);
 if(errors)fault(errors);
 if(state_.phase==PumpPhase::Fault||state_.phase==PumpPhase::Complete||state_.phase==PumpPhase::EvidenceFrozen)return request();
 if(config_.scenario==PumpScenario::Normal) { select(PumpPhase::Disabled,0,false,0);return request(); }
 if(now<state_.deadline_us)return request();
 if(now-state_.deadline_us>1000) { fault(PumpDeadlineLate);return request(); }
 if(!b.startup_ready||!b.capture_ready) { fault(PumpInitFailure);return request(); }
 const uint8_t phase=uint8_t(state_.phase);
 if(state_.phase==PumpPhase::InitialLow)select(PumpPhase::Step0,0,true,state_.t0_us+5000000+config_.plateau_us());
 else if(phase>=2&&phase<7) { static const uint8_t points[]={0,25,40,50,80,100};select(PumpPhase(phase+1),points[phase-1],true,state_.deadline_us+config_.plateau_us()); }
 else select(PumpPhase::Complete,0,false,state_.deadline_us);
 return request();
}
bool OutputPolicy::accuracy_collect(uint64_t now) const {
 if(config_.scenario!=PumpScenario::Accuracy||uint8_t(state_.phase)!=uint8_t(2+config_.trace_step))return false;
 return now>=state_.t0_us+5000000+config_.trace_step*config_.plateau_us()+1000000;
}
} // namespace bridge

namespace bridge {
static const uint64_t fnv_offset=0xcbf29ce484222325ULL;
uint64_t pump_hash_integer(uint64_t h,uint64_t value,unsigned bytes) {for(unsigned n=bytes;n;--n){h^=uint8_t(value>>((n-1)*8));h*=0x100000001b3ULL;}return h;}
OutputEvidence::OutputEvidence():header_{},records_{}{}
void OutputEvidence::identity(uint64_t fp,uint64_t boot,PumpScenario s){header_.fingerprint=fp;header_.boot=boot;header_.scenario=uint8_t(s);}
bool OutputEvidence::append(OutputRecord x) {
 if(header_.frozen)return true;
 if(header_.count==64){header_.overflow=true;return false;}
 x.seq=header_.count+1;records_[header_.count++]=x;header_.last_seq=x.seq;if(header_.count==1)header_.first_seq=x.seq;
 if(x.sdk_status&&header_.first_operation==0){header_.first_operation=x.kind;header_.first_error=x.sdk_status;}
 return true;
}
void OutputEvidence::fallback(PumpOperation op,int32_t status){if(header_.frozen)return;header_.fallback_attempted=true;if(!header_.fallback_error){header_.fallback_operation=uint8_t(op);header_.fallback_error=status;}}
uint64_t OutputEvidence::digest() const {
 uint64_t h=fnv_offset;
 for(unsigned i=0;i<header_.count;++i){const auto& r=records_[i];h=pump_hash_integer(h,r.seq,4);h=pump_hash_integer(h,r.kind,1);h=pump_hash_integer(h,r.duty,1);h=pump_hash_integer(h,r.applied,1);h=pump_hash_integer(h,r.reasons,1);h=pump_hash_integer(h,uint32_t(r.sdk_status),4);h=pump_hash_integer(h,r.before_us,8);h=pump_hash_integer(h,r.after_us,8);h=pump_hash_integer(h,r.deadline_us,8);}return h;
}
static void window_counters(uint32_t* target,const CaptureCounters& c){target[0]=c.batches;target[1]=c.candidates;target[2]=c.valid;target[3]=c.invalid;target[4]=c.dropped;target[5]=c.faults;target[6]=c.duplicate_edges;}
CaptureWindowEvidence::CaptureWindowEvidence():header_{},started_(false),postfreeze_invalid_(false),observed_{}{}
void CaptureWindowEvidence::start(uint64_t t0){if(started_)return;started_=true;if(t0>UINT64_MAX-606017000ULL){fail(WindowIncomplete|WindowBudgetMiss);header_.flags|=WindowFrozen;return;}header_.start_us=t0+5000000;header_.end_us=t0+606000000;}
void CaptureWindowEvidence::bump(unsigned i){if(header_.counts[i]==UINT32_MAX)fail(WindowSaturated);else ++header_.counts[i];}
void CaptureWindowEvidence::fail(uint32_t flags){if(frozen())postfreeze_invalid_=true;else header_.flags|=flags|WindowIncomplete;}
void CaptureWindowEvidence::observe(const CaptureSnapshot& s,uint64_t now){
 if(!started_||frozen())return;
 if(!header_.has_start_checkpoint&&now>=header_.start_us){header_.has_start_checkpoint=true;header_.start_checkpoint_us=now;window_counters(header_.baseline,s.counters);}
 const uint32_t before[]={observed_.dropped,observed_.faults,observed_.duplicate_edges};
 const uint32_t after[]={s.counters.dropped,s.counters.faults,s.counters.duplicate_edges};
 const unsigned indices[]={4,5,6};
 for(unsigned i=0;i<3;++i)if(after[i]!=before[i]){fail(WindowAssociationUnknown|(i==1?WindowCaptureFault:WindowLossOrOrderError));if(after[i]<before[i]||after[i]-before[i]>UINT32_MAX-header_.counts[indices[i]])fail(WindowSaturated);else header_.counts[indices[i]]+=after[i]-before[i];}
 observed_=s.counters;
 if(s.fault_bits||s.counters.saturated){fail(s.counters.saturated?WindowSaturated:WindowCaptureFault);}
 if(s.reported_at_us>=s.threshold_at_us&&s.threshold_at_us){header_.has_fault_delivery=true;const uint64_t delivery=s.reported_at_us-s.threshold_at_us+100000;if(delivery>header_.max_fault_delivery_us)header_.max_fault_delivery_us=delivery;}

}
void CaptureWindowEvidence::sample(const PulseMeasurement& p,uint64_t rearm,uint64_t processing){
 if(!started_)return;
 if(!p.complete){if(!frozen()){bump(3);fail(WindowAssociationUnknown);}return;}
 if(p.end_us<header_.start_us||p.end_us>=header_.end_us)return;
 if(frozen()){postfreeze_invalid_=true;return;}
 bump(0);
 if(p.status==CaptureStatus::Valid)bump(1);else bump(2);
 if(header_.has_samples&&p.end_us<=header_.last_end_us){bump(7);fail(WindowLossOrOrderError);}
 if(!header_.has_samples){header_.has_samples=true;header_.first_seq=p.seq;header_.first_epoch=p.epoch;header_.first_end_us=p.end_us;}
 else if(p.seq==header_.last_seq){bump(6);fail(WindowLossOrOrderError);}
 header_.last_seq=p.seq;header_.last_epoch=p.epoch;header_.last_end_us=p.end_us;
 if(rearm>header_.max_rearm_us)header_.max_rearm_us=rearm;
 if(processing>header_.max_processing_us)header_.max_processing_us=processing;
 if(rearm>1000||processing>10000)fail(WindowBudgetMiss);
}
void CaptureWindowEvidence::close(const CaptureSnapshot& s,uint64_t now,bool barrier,uint32_t seq,uint32_t epoch){
 if(!started_||frozen())return;
 observe(s,now);
 if(now<header_.end_us+6000)return;
 if(now>header_.end_us+17000)fail(WindowClosureLate);
 else if(!barrier)return;
 if(!barrier)fail(WindowAssociationUnknown);
 header_.has_end_checkpoint=true;header_.end_checkpoint_us=now;window_counters(header_.ending,s.counters);
 header_.finalized_us=now;header_.watermark_seq=seq;header_.watermark_epoch=epoch;header_.flags|=WindowFrozen;
}
uint64_t CaptureWindowEvidence::digest() const {
 uint64_t h=fnv_offset;
 for(auto x:header_.baseline)h=pump_hash_integer(h,x,4);
 for(auto x:header_.ending)h=pump_hash_integer(h,x,4);
 for(auto x:header_.counts)h=pump_hash_integer(h,x,4);
 for(auto x:{header_.first_seq,header_.last_seq,header_.first_epoch,header_.last_epoch})h=pump_hash_integer(h,x,4);
 h=pump_hash_integer(h,header_.first_end_us,8);h=pump_hash_integer(h,header_.last_end_us,8);
 h=pump_hash_integer(h,header_.max_rearm_us,8);h=pump_hash_integer(h,header_.max_processing_us,8);return pump_hash_integer(h,header_.max_fault_delivery_us,8);
}
}
