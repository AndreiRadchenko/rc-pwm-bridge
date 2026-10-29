#include "rc_capture_fixtures.hpp"
using namespace bridge;
int main() {
  Checks c;
  for (uint32_t w : {899u, 900u, 1000u, 2100u, 2101u}) {
    CaptureFixture f;
    f.pulse(1000, 1500);
    f.pulse(21000, w);
    auto s = f.policy.snapshot();
    CHECK(c, s.measurement.status == ((w < 900 || w > 2100)
                                          ? CaptureStatus::InvalidWidth
                                          : CaptureStatus::Valid));
    CHECK(c, s.control.has_valid == (w >= 900 && w <= 2100));
  }
  for (uint32_t p : {17999u, 18000u, 20000u, 22000u, 22001u}) {
    CaptureFixture f;
    f.pulse(1000, 1000);
    f.pulse(1000 + p, 1500);
    CHECK(c, f.policy.snapshot().measurement.status ==
                 ((p < 18000 || p > 22000) ? CaptureStatus::InvalidPeriod
                                           : CaptureStatus::Valid));
  }
  CaptureFixture f;
  f.pulse(1000, 1500);
  CHECK(c, !f.policy.snapshot().control.has_valid);
  f.pulse(20000, 1500);
  f.pulse(41000, 1500);
  CHECK(c, f.policy.snapshot().measurement.period_us == 21000);
  f.pulse(49000, 1500);
  CHECK(c,
        f.policy.snapshot().measurement.status == CaptureStatus::InvalidPeriod);
  f.pulse(69000, 1500);
  CHECK(c, f.policy.snapshot().measurement.status == CaptureStatus::Valid);
  CaptureFixture both;both.pulse(1000,1500);both.pulse(9000,899);
  auto validation=both.policy.snapshot().measurement.validation();
  CHECK(c,!validation.width_ok&&validation.period_known&&!validation.period_ok);
  CHECK(c,both.policy.snapshot().control.last_valid_time_us==0);
  return c.failures ? 1 : 0;
}
