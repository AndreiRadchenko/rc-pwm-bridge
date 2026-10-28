#ifndef PLATFORM_STARTUP_PORT_HPP
#define PLATFORM_STARTUP_PORT_HPP

#include <cstddef>
#include <cstdint>
#include "bridge/startup.hpp"

namespace platform {
struct ClockReading { bool valid; int64_t raw_us; };
struct OutputApplyResult { bool pwm_low_api_applied; };
struct DiagnosticSnapshot {
    uint64_t sequence;
    uint64_t profile_fingerprint;
    bridge::StartupSnapshot startup;
    bool pwm_low_api_applied;
};
static_assert(sizeof(DiagnosticSnapshot) <= 96, "snapshot exceeds transport bound");
struct ClockPort { virtual ClockReading read() = 0; virtual ~ClockPort() = default; };
struct ProfilePort { virtual bool matches() = 0; virtual ~ProfilePort() = default; };
struct OutputPort { virtual OutputApplyResult apply_low() = 0; virtual ~OutputPort() = default; };
struct SetupPort { virtual bool transport_ready() = 0; virtual bool worker_ready() = 0; virtual ~SetupPort() = default; };
struct DiagnosticPort { virtual void publish(const DiagnosticSnapshot&) = 0; virtual ~DiagnosticPort() = default; };
struct TransportPort { virtual std::size_t write(const char*, std::size_t) = 0; virtual ~TransportPort() = default; };
bool checked_clock(ClockReading reading, bridge::MonotonicUs& result);
class StartupPolicy {
public:
    StartupPolicy(ClockPort&, ProfilePort&, OutputPort&, SetupPort&, DiagnosticPort&, uint64_t fingerprint);
    void poll();
    bridge::StartupSnapshot snapshot() const { return supervisor_.snapshot(); }
private:
    void publish_if_changed(bool force = false);
    ClockPort& clock_; ProfilePort& profile_; OutputPort& output_; SetupPort& setup_; DiagnosticPort& diagnostic_;
    bridge::StartupSupervisor supervisor_;
    DiagnosticSnapshot last_;
    bool published_;
    bool low_;
};
class DiagnosticConsumer {
public:
    DiagnosticConsumer() : attempted_(false), sequence_(0) {}
    bool consume(const DiagnosticSnapshot&, TransportPort&);
private:
    bool attempted_; uint64_t sequence_;
};
}
#endif
