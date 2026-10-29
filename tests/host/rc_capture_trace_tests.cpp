#include "bridge/rc_capture_trace.hpp"
#include "test_support.hpp"
int main() {
  Checks c;
  bridge::AccuracyTrace t;
  bridge::CaptureSnapshot s{};
  s.max_processing_us = 600;
  s.max_rearm_us = 20;
  s.max_fault_lateness_us = 9000;
  s.measurement.complete = true;
  s.measurement.status = bridge::CaptureStatus::PeriodUnknown;
  for (unsigned i = 0; i < 128; ++i) {
    s.measurement.start_us = 1000 + i * 20000;
    s.measurement.end_us = s.measurement.start_us + 1500;
    s.measurement.seq = i;
    s.measurement.width_us = 1500;
    t.append(s, s.measurement.end_us + 5000);
  }
  CHECK(c, t.header().count == 128 && t.header().frozen && !t.header().failed);
  CHECK(c, t.record(0).flags ==
               uint32_t(bridge::CaptureStatus::PeriodUnknown) + 256);
  CHECK(c, t.header().max_processing_us == 600 &&
               t.header().max_rearm_us == 20 &&
               t.header().max_fault_lateness_us == 9000);
  auto last = t.record(127).start_us;
  t.append(s, 9999999);
  CHECK(c, t.record(127).start_us == last);
  bridge::AccuracyTrace gap;
  gap.append(s, 1);
  s.counters.dropped = 1;
  gap.append(s, 2);
  CHECK(c, gap.header().failed);
  bridge::AccuracyTrace no_candidate;
  s.counters.dropped = 0;
  no_candidate.append(s, 1);
  s.counters.dropped = 1;
  no_candidate.observe(s);
  CHECK(c, no_candidate.header().failed);
  return c.failures ? 1 : 0;
}
