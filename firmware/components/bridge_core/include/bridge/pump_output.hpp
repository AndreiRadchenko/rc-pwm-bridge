#ifndef BRIDGE_PUMP_OUTPUT_HPP
#define BRIDGE_PUMP_OUTPUT_HPP
#include "bridge/rc_capture.hpp"
#include <stddef.h>
#include <stdint.h>
namespace bridge {
enum class PumpMode : uint8_t { Normal=0, Bench=1 };
enum class PumpScenario : uint8_t { Normal=0, Waveform=1, Stress=2, Accuracy=3 };
enum class PumpPhase : uint8_t { Disabled=0, InitialLow=1, Step0=2, Step1=3, Step2=4, Step3=5, Step4=6, Step5=7, Complete=8, Fault=9, EvidenceFrozen=10 };
enum class PumpApplied : uint8_t { Unknown=0, Low=1, Pwm=2, High=3 };
enum class PumpOperation : uint8_t { None=0, SessionStart=1, Init=2, Set=3, Update=4, Stop=5, GpioLow=6, GpioDirection=7, Cleanup=8, Fault=9, Complete=10, EvidenceFrozen=11 };
enum PumpReason : uint8_t { PumpInitFailure=1, PumpInvalidRequest=2, PumpCaptureFault=4, PumpClockError=8, PumpApplyApiError=16, PumpLowUnconfirmed=32, PumpDeadlineLate=64, PumpEvidenceOverflow=128 };
struct BenchConfig {
  PumpScenario scenario;
  uint8_t trace_step;
  bool valid() const;
  uint64_t plateau_us() const;
  static BenchConfig normal();
  static BenchConfig bench(PumpScenario scenario, uint8_t trace_step=0);
};
struct BootContext { bool startup_ready, startup_had_failure, capture_ready; int32_t reset_reason; };
struct OutputRequest { uint64_t generation, deadline_us; uint8_t percent, reasons; bool permission; };
struct OutputSnapshot {
  uint64_t t0_us, deadline_us, generation;
  PumpMode mode;
  PumpScenario scenario;
  PumpPhase phase;
  uint8_t requested_percent;
  bool bench_permission;
  PumpApplied applied;
  uint8_t reasons;
};
class OutputPolicy {
public:
  explicit OutputPolicy(BenchConfig);
  OutputRequest tick(uint64_t now, BootContext, uint8_t faults=0);
  void fault(uint8_t reason);
  void freeze();
  void applied(PumpApplied value) { state_.applied=value; }
  OutputSnapshot snapshot() const { return state_; }
  OutputRequest request() const;
  BenchConfig config() const { return config_; }
  bool accuracy_collect(uint64_t now) const;
private:
  void select(PumpPhase,uint8_t,bool,uint64_t);
  const BenchConfig config_;
  OutputSnapshot state_;
  uint64_t last_now_,command_deadline_;
  bool started_;
};
bool pump_count(uint32_t percent,uint32_t& count);
struct OutputRecord {
  uint32_t seq;
  uint8_t kind, duty, applied, reasons;
  int32_t sdk_status;
  uint64_t before_us, after_us, deadline_us;
};
struct OutputHeader {
  uint64_t fingerprint, boot, t0_us;
  uint32_t count, first_seq, last_seq;
  int32_t first_error, fallback_error;
  uint8_t scenario, first_operation, fallback_operation;
  bool fallback_attempted, frozen, overflow;
};
class OutputEvidence {
public:
  OutputEvidence();
  void identity(uint64_t fingerprint,uint64_t boot,PumpScenario);
  bool append(OutputRecord);
  void freeze() { header_.frozen=true; }
  void fallback(PumpOperation,int32_t);
  void begin(uint64_t t0) { header_.t0_us=t0; }
  const OutputHeader& header() const { return header_; }
  const OutputRecord& record(size_t i) const { return records_[i]; }
  uint64_t digest() const;
private:
  OutputHeader header_;
  OutputRecord records_[64];
};
static_assert(sizeof(OutputRecord)<=40,"output native record budget");
static_assert(sizeof(OutputHeader)<=128,"output header budget");
static_assert(sizeof(OutputEvidence)<=2688,"output evidence RAM budget");
uint64_t pump_hash_integer(uint64_t hash,uint64_t value,unsigned bytes);
enum WindowFlag : uint32_t { WindowIncomplete=1,WindowSaturated=2,WindowClosureLate=4,WindowAssociationUnknown=8,WindowCaptureFault=16,WindowLossOrOrderError=32,WindowBudgetMiss=64,WindowFrozen=128 };
struct CaptureWindowHeader {
  uint64_t start_us,end_us,start_checkpoint_us,end_checkpoint_us,finalized_us;
  uint64_t first_end_us,last_end_us,max_rearm_us,max_processing_us,max_fault_delivery_us;
  uint32_t baseline[7], ending[7], counts[8];
  uint32_t first_seq,last_seq,first_epoch,last_epoch,watermark_seq,watermark_epoch,flags;
  bool has_start_checkpoint,has_end_checkpoint,has_samples,has_fault_delivery;
};
class CaptureWindowEvidence {
public:
  CaptureWindowEvidence();
  void start(uint64_t t0);
  void observe(const CaptureSnapshot&,uint64_t now);
  void sample(const PulseMeasurement&,uint64_t rearm_us,uint64_t processing_us);
  void close(const CaptureSnapshot&,uint64_t now,bool barrier,uint32_t seq,uint32_t epoch);
  void fail(uint32_t flags);
  const CaptureWindowHeader& header() const { return header_; }
  bool frozen() const { return (header_.flags&WindowFrozen)!=0; }
  bool postfreeze_invalid() const { return postfreeze_invalid_; }
  uint64_t digest() const;
private:
  void bump(unsigned index);
  CaptureWindowHeader header_;
  bool started_,postfreeze_invalid_;
  CaptureCounters observed_;
};
static_assert(sizeof(CaptureWindowEvidence)<=512,"stress evidence RAM budget");
} // namespace bridge
#endif
