#include "platform/pump_output.hpp"
#include "test_support.hpp"
#include <vector>
#include <utility>
#include <algorithm>
using namespace bridge; using namespace platform;
struct FakeSdk:PumpSdk {
 uint64_t time=1;uint32_t pending=0;int calls=0,fail_call=-1,inject_call=-1;bool fail_all=false;uint64_t delay=1;
 std::vector<std::pair<PumpOperation,uint32_t>> transcript;
 std::vector<int> failures;
 uint64_t now_us() override{return time;}
 uint32_t faults() override{return pending;}
 int32_t invoke(PumpOperation op,uint32_t v,const PumpHardwareConfig& cfg) override {
  if(cfg.pin!=0||cfg.clock_hz!=80000000||cfg.frequency_hz!=1000||cfg.resolution!=13||cfg.timer!=0||cfg.channel!=0||cfg.hpoint!=0)return -100;
  transcript.push_back({op,v});time+=delay;
  if(calls==inject_call)pending=1;
  const int index=calls++;return (index==fail_call||fail_all||std::find(failures.begin(),failures.end(),index)!=failures.end())?-7:0;
 }
};
int main(){Checks c;
 FakeSdk s; EspPumpOutput out(s,true);CHECK(c,out.initialize());
 unsigned points[]={0,25,40,50,80,100};uint64_t gen=0;
 for(unsigned p:points){auto q=OutputRequest{++gen,s.time,p?uint8_t(p):uint8_t(0),0,true};CHECK(c,out.apply(q));CHECK(c,out.applied()==(p==0?PumpApplied::Low:p==100?PumpApplied::High:PumpApplied::Pwm));}
 for(uint32_t p=1;p<100;++p){uint32_t expected;CHECK(c,pump_count(p,expected));CHECK(c,expected<8192);CHECK(c,out.apply({++gen,s.time,uint8_t(p),0,true}));CHECK(c,s.transcript[s.transcript.size()-2]==std::make_pair(PumpOperation::Set,expected));}
 auto n=s.calls;CHECK(c,out.apply({gen,s.time,99,0,true}));CHECK(c,s.calls==n);
 CHECK(c,!out.apply({++gen,s.time,101,0,true}));CHECK(c,out.applied()==PumpApplied::Low);CHECK(c,out.reasons()&PumpInvalidRequest);
 CHECK(c,!out.apply({++gen,s.time,100,0,true}));CHECK(c,out.applied()==PumpApplied::Low);
 FakeSdk normal;EspPumpOutput low(normal,false);CHECK(c,low.initialize());CHECK(c,!low.apply({1,1,100,0,true}));CHECK(c,low.applied()==PumpApplied::Low);
 for(int point=0;point<4;++point){FakeSdk f;f.fail_call=point;EspPumpOutput bad(f,true);CHECK(c,!bad.initialize());CHECK(c,bad.terminal());CHECK(c,bad.applied()==(point<2?PumpApplied::Unknown:PumpApplied::Low));CHECK(c,bool(bad.reasons()&PumpLowUnconfirmed)==(point<2));}
 FakeSdk fallback;EspPumpOutput recover(fallback,true);CHECK(c,recover.initialize());fallback.fail_call=fallback.calls;
 CHECK(c,!recover.apply({1,fallback.time,100,0,true}));CHECK(c,recover.applied()==PumpApplied::Low);CHECK(c,!(recover.reasons()&PumpLowUnconfirmed));
 FakeSdk all;EspPumpOutput failure(all,true);CHECK(c,failure.initialize());all.fail_all=true;CHECK(c,!failure.apply({1,all.time,100,0,true}));CHECK(c,failure.reasons()&PumpLowUnconfirmed);CHECK(c,failure.applied()==PumpApplied::Unknown);
 for(int between=0;between<2;++between){FakeSdk race;EspPumpOutput r(race,true);CHECK(c,r.initialize());race.inject_call=race.calls+between;CHECK(c,!r.apply({1,race.time,50,0,true}));CHECK(c,r.applied()==PumpApplied::Low);CHECK(c,r.reasons()&PumpCaptureFault);if(between==0){bool update=false;for(auto x:race.transcript)update|=x.first==PumpOperation::Update;CHECK(c,!update);}}
 FakeSdk before;EspPumpOutput noHigh(before,true);CHECK(c,noHigh.initialize());before.pending=1;CHECK(c,!noHigh.apply({1,before.time,100,0,true}));for(auto x:before.transcript)CHECK(c,!(x.first==PumpOperation::Stop&&x.second==1));
 FakeSdk slow;EspPumpOutput late(slow,true);CHECK(c,late.initialize());slow.delay=2001;CHECK(c,!late.apply({1,slow.time,25,0,true}));CHECK(c,late.reasons()&PumpDeadlineLate);

 for(int offset: {0,1}){FakeSdk f;OutputEvidence log;EspPumpOutput a(f,true,&log);CHECK(c,a.initialize());f.fail_call=f.calls+offset;CHECK(c,!a.apply({1,f.time,50,0,true}));CHECK(c,a.applied()==PumpApplied::Low);CHECK(c,log.header().first_error==-7);size_t end=f.transcript.size();a.apply({2,f.time,80,0,true});for(size_t i=end;i<f.transcript.size();++i)CHECK(c,f.transcript[i].first!=PumpOperation::Set&&f.transcript[i].first!=PumpOperation::Update&&!(f.transcript[i].first==PumpOperation::Stop&&f.transcript[i].second));}
 for(int fail_fallback: {-1,0,1}){FakeSdk f;OutputEvidence log;EspPumpOutput a(f,true,&log);CHECK(c,a.initialize());CHECK(c,a.apply({1,f.time,100,0,true}));int next=f.calls;f.failures.push_back(next);if(fail_fallback>=0)f.failures.push_back(next+1+fail_fallback);CHECK(c,!a.apply({2,f.time,0,0,false}));CHECK(c,bool(a.reasons()&PumpLowUnconfirmed)==(fail_fallback>=0));CHECK(c,a.applied()==(fail_fallback<0?PumpApplied::Low:PumpApplied::Unknown));CHECK(c,log.header().fallback_attempted);f.failures.clear();a.apply({3,f.time,80,0,true});CHECK(c,a.applied()==PumpApplied::Low);CHECK(c,bool(a.reasons()&PumpLowUnconfirmed)==(fail_fallback>=0));}
 for(int offset:{1,2}){FakeSdk f;OutputEvidence log;EspPumpOutput a(f,true,&log);CHECK(c,a.initialize());f.fail_call=f.calls+offset;a.cleanup();CHECK(c,a.reasons()&PumpApplyApiError);CHECK(c,a.applied()==PumpApplied::Low);CHECK(c,log.header().first_error==-7);}
 FakeSdk mismatch;EspPumpOutput same(mismatch,true);CHECK(c,same.initialize());CHECK(c,same.apply({1,mismatch.time,25,0,true}));CHECK(c,!same.apply({1,mismatch.time,80,0,true}));CHECK(c,same.reasons()&PumpInvalidRequest);CHECK(c,same.applied()==PumpApplied::Low);
 FakeSdk timestamp;EspPumpOutput cutoff(timestamp,true);CHECK(c,cutoff.initialize());CHECK(c,cutoff.apply({1,timestamp.time,100,0,true}));timestamp.time=9000;CHECK(c,!cutoff.apply({2,7000,0,PumpCaptureFault,false}));CHECK(c,cutoff.reasons()&PumpDeadlineLate);
 FakeSdk replay;EspPumpOutput barrier(replay,true);CHECK(c,barrier.initialize());CHECK(c,barrier.apply({6,replay.time,80,0,true}));CHECK(c,barrier.apply({2,replay.time,0,0,false}));CHECK(c,!barrier.apply({4,replay.time,50,0,true}));CHECK(c,barrier.applied()==PumpApplied::Low);
 FakeSdk wrap;EspPumpOutput wrapping(wrap,true);CHECK(c,wrapping.initialize());CHECK(c,wrapping.apply({UINT64_MAX,wrap.time,25,0,true}));CHECK(c,!wrapping.apply({0,wrap.time,80,0,true}));CHECK(c,wrapping.applied()==PumpApplied::Low);
 FakeSdk overflow;OutputEvidence full;for(unsigned i=0;i<64;++i)full.append({});EspPumpOutput overflowing(overflow,true,&full);CHECK(c,!overflowing.initialize());CHECK(c,overflowing.reasons()&PumpEvidenceOverflow);CHECK(c,overflowing.applied()==PumpApplied::Low);for(auto x:overflow.transcript)CHECK(c,x.first!=PumpOperation::Set&&x.first!=PumpOperation::Update);
 return c.failures?1:0;}
