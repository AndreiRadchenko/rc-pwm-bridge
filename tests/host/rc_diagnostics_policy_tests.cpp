#include "platform/startup_port.hpp"
#include "test_support.hpp"
int main() {
  Checks c;
  platform::RcDiagnosticPolicy p;
  platform::DiagnosticSnapshot s{};
  p.publish(s);
  auto v = p.copy().sequence;
  p.publish(s);
  CHECK(c, p.overwritten() == 1);
  p.ack(v);
  CHECK(c, p.pending());
  p.ack(p.copy().sequence);
  CHECK(c, !p.pending());
  CHECK(c, p.slot(0, true));
  CHECK(c, !p.slot(99999, true));
  CHECK(c, !p.slot(100000, false) && p.busy() == 1);
  CHECK(c, !p.slot(199999, true));
  CHECK(c, p.slot(200000, true));
  p.written(0, 10);
  p.written(5, 10);
  CHECK(c, p.drops() == 2 && p.attempts() == 3 && p.bytes() == 5);
  bool sat = false;
  uint32_t x = UINT32_MAX;
  bridge::increment(x, sat);
  CHECK(c, sat && x == UINT32_MAX);
  s.has_output=true;s.output.mode=bridge::PumpMode::Bench;s.output.phase=bridge::PumpPhase::Step4;s.output.requested_percent=80;s.output.bench_permission=true;s.output.applied=bridge::PumpApplied::Pwm;s.capture.control.calculated_duty_percent=40;
  p.publish(s);auto pair=p.copy();CHECK(c,pair.output.requested_percent==80&&pair.capture.control.calculated_duty_percent==40&&!pair.capture.control.work_permitted);
  const auto version=pair.sequence;p.publish(s);p.ack(version);CHECK(c,p.pending());
  s.output.phase=bridge::PumpPhase::Fault;s.output.bench_permission=false;s.output.requested_percent=0;s.output.reasons=bridge::PumpCaptureFault;p.publish(s);
  CHECK(c,p.copy().output.reasons==bridge::PumpCaptureFault);CHECK(c,!p.slot(200001,true));CHECK(c,p.slot(300000,true));
  return c.failures ? 1 : 0;
}
