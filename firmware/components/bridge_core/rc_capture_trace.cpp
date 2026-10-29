#include "bridge/rc_capture_trace.hpp"
namespace bridge {
void AccuracyTrace::observe(const CaptureSnapshot &s) {
  if (!header_.count || header_.frozen)
    return;
  header_.max_rearm_us = s.max_rearm_us;
  header_.max_processing_us = s.max_processing_us;
  header_.max_fault_lateness_us = s.max_fault_lateness_us;
  header_.end_drops = s.counters.dropped;
  header_.end_faults = s.counters.faults;
  header_.failed = header_.failed || s.counters.saturated ||
                   header_.end_drops != header_.base_drops ||
                   header_.end_faults != header_.base_faults;
}
void AccuracyTrace::append(const CaptureSnapshot &s, uint64_t now) {
  if (header_.frozen)
    return;
  header_.max_rearm_us = s.max_rearm_us;
  header_.max_processing_us = s.max_processing_us;
  header_.max_fault_lateness_us = s.max_fault_lateness_us;
  const PulseMeasurement &p = s.measurement;
  if (!header_.count) {
    header_.started_us = p.start_us;
    header_.base_drops = s.counters.dropped;
    header_.base_faults = s.counters.faults;
  }
  header_.end_drops = s.counters.dropped;
  header_.end_faults = s.counters.faults;
  header_.failed = header_.failed || s.counters.saturated || !p.complete ||
                   p.status == CaptureStatus::Error ||
                   header_.end_drops != header_.base_drops ||
                   header_.end_faults != header_.base_faults;
  records_[header_.count++] = {p.start_us,
                               p.end_us,
                               now,
                               p.seq,
                               p.epoch,
                               p.width_us,
                               p.period_us,
                               uint32_t(p.status) | (p.complete ? 256u : 0u) |
                                   (p.period_known ? 512u : 0u),
                               s.fault_bits};
  header_.ended_us = p.end_us;
  header_.frozen = header_.count == 128;
}
} // namespace bridge
