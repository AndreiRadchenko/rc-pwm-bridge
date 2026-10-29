#ifndef BRIDGE_RC_CAPTURE_TRACE_HPP
#define BRIDGE_RC_CAPTURE_TRACE_HPP
#include "bridge/rc_capture.hpp"
namespace bridge {
struct TraceRecord {
  uint64_t start_us, end_us, accepted_at_us;
  uint32_t seq, epoch, width_us, period_us, flags, fault_bits;
};
static_assert(sizeof(TraceRecord) <= 48, "trace record budget");
struct TraceHeader {
  uint64_t started_us, ended_us;
  uint64_t max_rearm_us, max_processing_us, max_fault_lateness_us;
  uint32_t count, base_drops, end_drops, base_faults, end_faults;
  bool frozen, failed;
};
class AccuracyTrace {
public:
  AccuracyTrace() : header_{}, records_{} {}
  void observe(const CaptureSnapshot &);
  void append(const CaptureSnapshot &, uint64_t now);
  const TraceHeader &header() const { return header_; }
  const TraceRecord &record(size_t i) const { return records_[i]; }

private:
  TraceHeader header_;
  TraceRecord records_[128];
};
static_assert(sizeof(TraceHeader) <= 256, "trace metadata budget");
static_assert(sizeof(AccuracyTrace) <= 6400, "trace RAM budget");
} // namespace bridge
#endif
