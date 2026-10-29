#include "rc_capture_fixtures.hpp"
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
  return c.failures ? 1 : 0;
}
