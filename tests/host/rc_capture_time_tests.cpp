#include "rc_capture_fixtures.hpp"
using namespace bridge;
int main() {
  Checks c;
  for (bool high : {false, true}) {
    CaptureFixture f(0, high);
    f.policy.poll({0, 0, 0, high, true, true}, 99999);
    CHECK(c, !f.policy.snapshot().fault_bits);
    f.policy.poll({0, 0, 0, high, true, true}, 100000);
    auto s = f.policy.snapshot();
    CHECK(c, s.fault_bits & (high ? StuckHigh : StuckLow));
    auto n = s.counters.faults;
    f.policy.poll({0, 0, 0, high, true, true}, 101000);
    CHECK(c, f.policy.snapshot().counters.faults == n);
  }
  CaptureFixture ended(0, true);
  ended.policy.on_edge({105000, 1, 1, false}, 109000);
  CHECK(c, ended.policy.snapshot().fault_bits & StuckHigh);
  CHECK(c, ended.policy.snapshot().max_fault_lateness_us == 9000);
  CaptureFixture late;
  late.policy.poll({0, 0, 0, false, true, true}, 110001);
  CHECK(c, late.policy.snapshot().fault_bits & DeadlineMissed);
  CaptureFixture guard;
  guard.policy.on_edge({1000, 1, 1, true}, 1000);
  guard.policy.poll({0, 1000, 1, true, true, true}, 12999);
  CHECK(c, !guard.policy.snapshot().fault_bits);
  guard.policy.poll({0, 1000, 1, true, true, true}, 13000);
  CHECK(c, guard.policy.snapshot().fault_bits & GuardExpired);
  CaptureFixture delay;
  auto b = delay.edges(1000, 1500);
  delay.policy.on_batch(b, b.completed_at_us + 10001);
  CHECK(c, delay.policy.snapshot().fault_bits & ProcessingLate);
  CaptureFixture wrap;
  wrap.seq = 0xfffffffeu;
  wrap.policy.recovered(0, false, wrap.seq);
  wrap.pulse(0x100000000ULL + 1000, 1500);
  wrap.pulse(0x100000000ULL + 21000, 1500);
  CHECK(c, wrap.policy.snapshot().control.has_valid);
  wrap.policy.poll({0, 0, 0, false, false, true}, 1);
  CHECK(c, wrap.policy.snapshot().fault_bits & ClockError);
  CaptureFixture boundary;
  boundary.policy.poll({0, 0, 0, false, true, true}, 110000);
  CHECK(c, (boundary.policy.snapshot().fault_bits & StuckLow) &&
               !(boundary.policy.snapshot().fault_bits & DeadlineMissed));
  for (unsigned gap : {1000u, 1001u}) {
    CaptureFixture f;
    auto x = f.edges(1000, 1500);
    x.rearmed_at_us = x.completed_at_us + gap;
    bool ok = f.policy.on_batch(x, x.rearmed_at_us);
    CHECK(c, ok == (gap == 1000));
  }
  CaptureFixture exact;
  auto x = exact.edges(1000, 1500);
  CHECK(c, exact.policy.on_batch(x, x.completed_at_us + 10000));
  CaptureFixture end_budget;
  auto final = end_budget.edges(1000, 1500);
  CHECK(c, end_budget.policy.on_batch(final, final.completed_at_us + 9999));
  end_budget.policy.processing_complete(final.completed_at_us,
                                        final.completed_at_us + 10001);
  CHECK(c, end_budget.policy.snapshot().fault_bits & ProcessingLate);
  CHECK(c,wrap.policy.snapshot().control.state==ControlState::Fault);
  CHECK(c,wrap.policy.snapshot().control.last_valid_time_us==0x100000000ULL+22500);
  CaptureFixture startup_high(0,true);
  CHECK(c,startup_high.policy.snapshot().phase==CaptureState::WaitingLow);
  CHECK(c,startup_high.policy.snapshot().measurement.status==CaptureStatus::Incomplete);
  startup_high.policy.poll({0,0,0,true,true,true},100000);
  CHECK(c,startup_high.policy.snapshot().phase==CaptureState::Resync);
  CHECK(c,startup_high.policy.snapshot().first_fault_at_us==100000);
  startup_high.policy.fault(RxError,101000);startup_high.policy.recovered(101000,false);
  CHECK(c,startup_high.policy.snapshot().first_fault_at_us==100000&&startup_high.policy.snapshot().last_fault_at_us==101000);
  CHECK(c,startup_high.policy.snapshot().phase==CaptureState::WaitingRise);
  return c.failures ? 1 : 0;
}
