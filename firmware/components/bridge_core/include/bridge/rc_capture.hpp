#ifndef BRIDGE_RC_CAPTURE_HPP
#define BRIDGE_RC_CAPTURE_HPP
#include "bridge/control.hpp"
#include <stddef.h>
#include <stdint.h>
namespace bridge {
enum class CaptureStatus : uint8_t {
  None,
  Valid,
  PeriodUnknown,
  InvalidWidth,
  InvalidPeriod,
  Incomplete,
  Error
};
enum class CaptureState : uint8_t { WaitingRise, WaitingLow, Receiving, Resync };
struct ValidationResult {
  CaptureStatus status;
  bool width_ok, period_known, period_ok;
};
enum CaptureFault : uint32_t {
  QueueLoss = 1u,
  ShapeError = 2u,
  Saturation = 4u,
  ClockError = 8u,
  RxError = 16u,
  StuckHigh = 32u,
  StuckLow = 64u,
  GuardExpired = 128u,
  ProcessingLate = 256u,
  RearmLate = 512u,
  DeadlineMissed = 1024u
};
// Times use one monotonic microsecond domain; seq/epoch wrap modulo 2^32.
struct EdgeEvent {
  uint64_t time_us;
  uint32_t seq, epoch;
  bool high;
};
struct Run {
  uint16_t duration_us;
  bool high;
};
struct RxBatch {
  uint64_t armed_at_us, completed_at_us, rearmed_at_us;
  uint32_t epoch, arm_seq, close_seq;
  uint16_t symbol_count, run_count;
  bool final_high;
  Run runs[96];
};
struct LineObservation {
  uint64_t started_us, changed_us;
  uint32_t seq;
  bool high, continuous, stable;
};
// Width is independent RMT duration; start/end are estimated edge times.
// Acceptance: complete width 900..2100 us and known period 18000..22000 us.
struct PulseMeasurement {
  uint64_t start_us, end_us;
  uint32_t width_us, period_us, seq, epoch;
  CaptureStatus status;
  bool complete, period_known;
  ValidationResult validation() const {
    return {status, complete && width_us >= 900 && width_us <= 2100,
            period_known, period_known && period_us >= 18000 && period_us <= 22000};
  }
};
struct CaptureCounters {
  uint32_t batches, candidates, valid, invalid, dropped, faults, duplicate_edges;
  bool saturated;
};
struct CaptureSnapshot {
  PulseMeasurement measurement;
  ControlSnapshot control;
  CaptureCounters counters;
  uint64_t started_at_us, threshold_at_us, reported_at_us,
      max_fault_lateness_us;
  uint64_t max_processing_us, max_rearm_us;
  uint64_t first_fault_at_us, last_fault_at_us;
  uint32_t fault_bits;
  bool recovery_needed;
  CaptureState phase;
};
void increment(uint32_t &value, bool &saturated);
class CapturePolicy {
public:
  CapturePolicy();
  void start(uint64_t now, bool high);
  void on_edge(const EdgeEvent &, uint64_t now);
  bool on_batch(const RxBatch &, uint64_t now);
  void processing_complete(uint64_t completed_at_us, uint64_t now);
  void poll(const LineObservation &, uint64_t now);
  void fault(uint32_t bits, uint64_t now, uint32_t dropped = 0);
  void recovered(uint64_t now, bool high);
  void recovered(uint64_t now, bool high, uint32_t sequence);
  CaptureSnapshot snapshot() const;

private:
  struct SavedEdge {
    EdgeEvent edge;
    uint32_t period;
    bool known;
  };
  SavedEdge edges_[64];
  size_t count_;
  ControlCore core_;
  CaptureSnapshot state_;
  uint64_t last_time_, barrier_, origin_, level_since_, guard_start_;
  uint32_t last_seq_, guard_epoch_;
  bool has_time_, has_seq_, has_origin_, high_, episode_reported_,
      guard_active_;
  bool clock_ok(uint64_t now);
  void episode(uint64_t end, uint64_t now);
};
enum class CaptureResource : uint8_t { Rmt, Gpio, Timer, Worker };
struct CaptureResourcePort {
  virtual bool acquire(CaptureResource) = 0;
  virtual void release(CaptureResource) = 0;
  virtual ~CaptureResourcePort() {}
};
class CaptureLifecycle {
public:
  CaptureLifecycle() : owned_(0), retry_at_(0) {}
  bool setup(CaptureResourcePort &, uint64_t now);
  void cleanup(CaptureResourcePort &);
  uint8_t owned() const { return owned_; }
  uint64_t retry_at() const { return retry_at_; }

private:
  uint8_t owned_;
  uint64_t retry_at_;
};
} // namespace bridge
#endif
