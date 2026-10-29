#include "rc_capture_fixtures.hpp"
using namespace bridge;
struct Resources : CaptureResourcePort {
  int fail, live[4] = {}, calls = 0;
  explicit Resources(int f) : fail(f) {}
  bool acquire(CaptureResource r) override {
    int i = int(r);
    ++calls;
    if (i == fail)
      return false;
    ++live[i];
    return true;
  }
  void release(CaptureResource r) override { --live[int(r)]; }
};
int main() {
  Checks c;
  for (int step = 0; step < 4; ++step) {
    Resources p(step);
    CaptureLifecycle life;
    CHECK(c, !life.setup(p, 0) && !life.owned());
    for (int n : p.live)
      CHECK(c, n == 0);
    int calls = p.calls;
    CHECK(c, !life.setup(p, 99999) && p.calls == calls);
    p.fail = -1;
    CHECK(c, life.setup(p, 100000) && life.owned() == 15);
    calls = p.calls;
    CHECK(c, life.setup(p, 200000) && p.calls == calls);
    for (int n : p.live)
      CHECK(c, n == 1);
    life.cleanup(p);
    for (int n : p.live)
      CHECK(c, n == 0);
  }
  CaptureFixture f;
  f.policy.poll({0, 0, 9, false, true, false}, 200000);
  CHECK(c, !f.policy.snapshot().fault_bits);
  f.policy.fault(QueueLoss, 200000);
  f.policy.recovered(200000, false);
  f.policy.poll({200000, 0, 9, false, false, true}, 200001);
  CHECK(c, !(f.policy.snapshot().fault_bits & StuckLow));
  f.pulse(201000, 1000);
  f.pulse(221000, 1500);
  CHECK(c, f.policy.snapshot().control.last_valid_time_us == 222500);
  CHECK(c, f.policy.snapshot().control.state == ControlState::Fault);
  CaptureFixture ordering;
  auto b = ordering.edges(1000, 1500);
  ordering.policy.on_edge({10000, ++ordering.seq, 2, true}, 10000);
  CHECK(c, ordering.policy.on_batch(b, 11000));
  ordering.policy.poll({0, 10000, ordering.seq, true, true, true}, 22000);
  CHECK(c, ordering.policy.snapshot().fault_bits & GuardExpired);
  CaptureFixture barrier;
  auto old = barrier.edges(1000, 1500);
  barrier.policy.fault(RxError, 6000);
  CHECK(c, !barrier.policy.on_batch(old, 7500));
  CHECK(c, !barrier.policy.snapshot().control.has_valid);
  CaptureFixture gap;
  gap.policy.on_edge({1000, 1, 1, true}, 1000);
  gap.policy.on_edge({2500, 3, 1, false}, 2500);
  CHECK(c, gap.policy.snapshot().fault_bits & ShapeError);

  CaptureFixture race;
  race.policy.poll({0, 1000, 1, true, true, true}, 1000);
  race.policy.on_edge({1000, 1, 1, true}, 1001);
  CHECK(c, !race.policy.snapshot().fault_bits);
  return c.failures ? 1 : 0;
}
