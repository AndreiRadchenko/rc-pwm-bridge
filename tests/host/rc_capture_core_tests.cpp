#include "rc_capture_fixtures.hpp"
#include "bridge/pump_output.hpp"
using namespace bridge;
int main() {
  Checks c;
  const uint32_t w[] = {1000, 1250, 1400, 1500, 1800, 2000};
  const unsigned d[] = {0, 25, 40, 50, 80, 100};
  for (unsigned i = 0; i < 6; ++i) {
    CaptureFixture f;
    f.pulse(1000, 1000);
    f.pulse(21000, w[i]);
    auto s = f.policy.snapshot();
    CHECK(c, s.control.calculated_duty_percent == d[i]);
    CHECK(c,
          !s.control.work_permitted && s.control.effective_duty_percent == 0);
    CHECK(c, s.control.last_valid_time_us == 21000 + w[i]);
    f.pulse(41000, 2101);
    auto b = f.policy.snapshot();
    CHECK(c, b.control.calculated_duty_percent == d[i] &&
                 b.control.last_valid_time_us == s.control.last_valid_time_us);
  }
  for (uint32_t width : {1004u, 1005u, 1006u, 1994u, 1995u}) {
    CaptureFixture f;
    f.pulse(1000, 1000);
    f.pulse(21000, width);
    CHECK(c, f.policy.snapshot().control.calculated_duty_percent ==
                 (width - 1000 + 5) / 10);
  }
  // The output policy sees explicit faults, never a calculated RC duty as permission.
  CaptureFixture capture;OutputPolicy output(BenchConfig::bench(PumpScenario::Waveform));
  output.tick(0,{true,false,true});output.tick(5000000,{true,false,true});output.tick(15000000,{true,false,true});
  for(unsigned i=0;i<800;++i)capture.pulse(1000+i*20000,1000);
  for(unsigned i=0;i<6;++i){capture.pulse(16000000+i*20000,w[i]);output.tick(16007000+i*20000,{true,false,true});CHECK(c,output.snapshot().requested_percent==25);}
  const auto last=capture.policy.snapshot().control.last_valid_time_us;
  capture.pulse(16120000,899);CHECK(c,capture.policy.snapshot().control.last_valid_time_us==last);CHECK(c,!capture.policy.snapshot().fault_bits);
  capture.policy.fault(QueueLoss,16130000);output.tick(16130000,{true,false,true},PumpCaptureFault);
  capture.policy.recovered(16130000,false,capture.seq);capture.pulse(16140000,1800);capture.pulse(16160000,1800);CHECK(c,capture.policy.snapshot().control.calculated_duty_percent==80);CHECK(c,!capture.policy.snapshot().control.work_permitted);
  output.tick(16170000,{true,false,true});CHECK(c,!output.request().permission&&output.request().percent==0);
  return c.failures ? 1 : 0;
}
