#include "rc_capture_fixtures.hpp"
using namespace bridge;
int main() {
  Checks c;
  for (int d : {-51, -50, 0, 50, 51}) {
    CaptureFixture f;
    auto b = f.edges(1000, 1500, d);
    CHECK(c, f.policy.on_batch(b, b.completed_at_us) == (d >= -50 && d <= 50));
  }
  CaptureFixture f;
  f.pulse(1000, 1500);
  auto b = f.edges(21000, 1800);
  CHECK(c, f.policy.on_batch(b, b.completed_at_us + 9999));
  auto s = f.policy.snapshot();
  CHECK(c, s.measurement.start_us == 21000 &&
               s.control.last_valid_time_us == 22800);
  CHECK(c, s.measurement.width_us == 1800);
  CaptureFixture bad;
  auto bb = bad.edges(1000, 1500);
  ++bb.epoch;
  CHECK(c, !bad.policy.on_batch(bb, bb.completed_at_us));
  CHECK(c, bad.policy.snapshot().fault_bits & ShapeError);
  CaptureFixture lv;
  auto x = lv.edges(1000, 1500);
  x.runs[0].high = false;
  CHECK(c, !lv.policy.on_batch(x, x.completed_at_us));
  return c.failures ? 1 : 0;
}
