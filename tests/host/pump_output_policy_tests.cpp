#include "bridge/pump_output.hpp"
#include "test_support.hpp"
#include <initializer_list>
using namespace bridge;
int main() {
 Checks c; const BootContext ready={true,false,true};
 OutputPolicy normal(BenchConfig::normal());
 for(uint64_t t=0;t<10000000;t+=1000) { auto q=normal.tick(t,ready);CHECK(c,!q.permission&&q.percent==0); }
 normal.tick(10000000,ready,PumpCaptureFault);CHECK(c,normal.snapshot().phase==PumpPhase::Fault);
 for(PumpScenario mode : {PumpScenario::Waveform,PumpScenario::Stress}) {
  OutputPolicy p(BenchConfig::bench(mode)); const uint64_t plateau=mode==PumpScenario::Stress?100000000:10000000;
  CHECK(c,p.tick(17,ready).percent==0);CHECK(c,p.snapshot().phase==PumpPhase::InitialLow);
  CHECK(c,!p.tick(5000016,ready).permission);
  unsigned points[]={0,25,40,50,80,100};
  for(unsigned i=0;i<6;++i){auto q=p.tick(5000017+i*plateau,ready);CHECK(c,q.permission&&q.percent==points[i]);CHECK(c,q.deadline_us==5000017+i*plateau);}
  auto q=p.tick(5000017+6*plateau,ready);CHECK(c,!q.permission&&q.percent==0);
  CHECK(c,p.snapshot().phase==PumpPhase::Complete);
  p.tick(700000000,ready);CHECK(c,!p.request().permission);
 }
 for(uint64_t late : {999ULL,1000ULL,1001ULL}) { OutputPolicy p(BenchConfig::bench(PumpScenario::Waveform));p.tick(0,ready);p.tick(5000000+late,ready);CHECK(c,(p.snapshot().phase==PumpPhase::Fault)==(late>1000)); }
 OutputPolicy simultaneous(BenchConfig::bench(PumpScenario::Waveform));simultaneous.tick(0,ready);simultaneous.tick(5000000,ready,PumpCaptureFault);CHECK(c,simultaneous.snapshot().phase==PumpPhase::Fault);
 simultaneous.tick(5000001,ready);CHECK(c,!simultaneous.request().permission);
 OutputPolicy retry(BenchConfig::bench(PumpScenario::Stress));retry.tick(0,{true,true,true});retry.tick(5000000,ready);CHECK(c,!retry.request().permission);
 OutputPolicy readiness(BenchConfig::bench(PumpScenario::Waveform));readiness.tick(0,{false,false,false});readiness.tick(5000000,{false,false,false});CHECK(c,readiness.snapshot().reasons&PumpInitFailure);
 OutputPolicy clock(BenchConfig::bench(PumpScenario::Waveform));clock.tick(900,ready);clock.tick(899,ready);CHECK(c,clock.snapshot().reasons&PumpClockError);
 OutputPolicy overflow(BenchConfig::bench(PumpScenario::Stress));overflow.tick(UINT64_MAX-10,ready);CHECK(c,overflow.snapshot().reasons&PumpClockError);
 OutputPolicy invalid(BenchConfig::bench(PumpScenario::Accuracy,6));invalid.tick(0,ready);CHECK(c,invalid.snapshot().reasons&PumpInvalidRequest);
 OutputPolicy accuracy(BenchConfig::bench(PumpScenario::Accuracy,0));accuracy.tick(0,ready);accuracy.tick(5000000,ready);CHECK(c,!accuracy.accuracy_collect(5999999));CHECK(c,accuracy.accuracy_collect(6000000));accuracy.freeze();CHECK(c,!accuracy.request().permission&&accuracy.snapshot().phase==PumpPhase::EvidenceFrozen);
 uint32_t count=999;CHECK(c,!pump_count(101,count));CHECK(c,count==999);CHECK(c,pump_count(40,count)&&count==3277);
 for(int32_t reason=0;reason<32;++reason){OutputPolicy boot(BenchConfig::bench(PumpScenario::Waveform));boot.tick(0,{true,false,true,reason});CHECK(c,boot.tick(5000000,{true,false,true,reason}).permission);CHECK(c,boot.snapshot().phase==PumpPhase::Step0);}
 return c.failures?1:0;
}
