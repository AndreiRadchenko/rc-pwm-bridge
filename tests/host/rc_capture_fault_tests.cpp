#include "rc_capture_fixtures.hpp"
using namespace bridge;
int main() {
  Checks c;
  CaptureFixture sat;
  auto b = sat.edges(1000, 1500);
  b.symbol_count = 48;
  CHECK(c, !sat.policy.on_batch(b, b.completed_at_us));
  CHECK(c, sat.policy.snapshot().fault_bits & Saturation);
  CaptureFixture over;
  over.policy.on_edge({1000, 1, 1, true}, 1000);
  RxBatch x{};
  x.armed_at_us = 1;
  x.completed_at_us = 6000;
  x.epoch = 1;
  x.close_seq = 1;
  x.final_high = true;
  x.symbol_count = 1;
  x.run_count = 1;
  x.runs[0] = {5000, true};
  CHECK(c, over.policy.on_batch(x, 6000));
  CHECK(c,
        over.policy.snapshot().measurement.status == CaptureStatus::Incomplete);
  over.policy.poll({0, 1000, 1, true, true, true}, 14000);
  CHECK(c, !(over.policy.snapshot().fault_bits & GuardExpired));
  CaptureFixture q;
  for (unsigned i = 1; i <= 65; ++i)
    q.policy.on_edge({i * 1000u, i, 1, bool(i % 2)}, i * 1000u);
  CHECK(c, q.policy.snapshot().fault_bits & QueueLoss);
  CaptureFixture f;
  f.pulse(1000, 1000);
  f.pulse(21000, 1800);
  f.policy.fault(RxError, 30000);
  f.policy.recovered(30000, false);
  f.pulse(41000, 2000);
  f.pulse(61000, 2000);
  auto s = f.policy.snapshot();
  CHECK(c, s.control.state == ControlState::Fault &&
               s.control.calculated_duty_percent == 100);
  CHECK(c, s.control.last_valid_time_us == 63000 && !s.control.work_permitted);
  CaptureFixture shape;
  auto a = shape.edges(1000, 1500);
  a.run_count = 3;
  a.runs[2] = {1, true};
  CHECK(c, !shape.policy.on_batch(a, a.completed_at_us));
  CaptureFixture losses;
  losses.policy.fault(QueueLoss, 10, 7);
  CHECK(c, losses.policy.snapshot().counters.dropped == 7);
  losses.policy.fault(QueueLoss, 20, UINT32_MAX);
  CHECK(c, losses.policy.snapshot().counters.dropped == UINT32_MAX &&
               losses.policy.snapshot().counters.saturated);
  CaptureFixture duplicate;duplicate.policy.on_edge({1000,1,1,true},1000);duplicate.policy.on_edge({1001,1,1,true},1001);CHECK(c,duplicate.policy.snapshot().counters.duplicate_edges==1);CHECK(c,duplicate.policy.snapshot().fault_bits&ShapeError);
  CaptureFixture truncated;auto truncated_batch=truncated.edges(1000,1500);truncated_batch.run_count=1;
  CHECK(c,!truncated.policy.on_batch(truncated_batch,truncated_batch.completed_at_us));
  CHECK(c,truncated.policy.snapshot().fault_bits&ShapeError);
  CaptureFixture future;auto future_batch=future.edges(1000,1500);future_batch.rearmed_at_us=future_batch.completed_at_us+1;
  CHECK(c,!future.policy.on_batch(future_batch,future_batch.completed_at_us));CHECK(c,future.policy.snapshot().fault_bits&ClockError);
  return c.failures ? 1 : 0;
}
