#ifndef PLATFORM_STARTUP_PORT_HPP
#define PLATFORM_STARTUP_PORT_HPP

#include <cstddef>
#include <cstdint>
#include "bridge/startup.hpp"
#include "bridge/rc_capture.hpp"

namespace platform {
struct ClockReading { bool valid; int64_t raw_us; };
struct OutputApplyResult { bool pwm_low_api_applied; };
struct DiagnosticTransportBuffers {
    // Pinned USB Serial/JTAG driver requires RX capacity greater than 64 B.
    static constexpr std::size_t rx_bytes = 128;
    static constexpr std::size_t tx_bytes = 512;
};
struct DiagnosticSnapshot {
    uint64_t sequence;
    uint64_t profile_fingerprint;
    bridge::StartupSnapshot startup;
    bool pwm_low_api_applied;
    bridge::CaptureSnapshot capture;
    bool has_capture;
};
static_assert(sizeof(DiagnosticSnapshot) <= 384, "snapshot exceeds transport bound");
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

// Caller serializes publish/copy/ack. Formatting is deliberately outside the lock.
class RcDiagnosticPolicy {
public:
    RcDiagnosticPolicy(): latest_{}, pending_(false), version_(0), last_slot_(0),
        attempted_(false), overwritten_(0), busy_(0), drops_(0), attempts_(0), bytes_(0), saturated_(false) {}
    void publish(const DiagnosticSnapshot&);
    DiagnosticSnapshot copy() const { return latest_; }
    void ack(uint64_t version);
    bool slot(uint64_t now, bool drained);
    void written(size_t actual,size_t expected);
    int format(const DiagnosticSnapshot&,char* buffer,size_t size,bool padded=false) const;
    uint32_t overwritten() const { return overwritten_; }
    uint32_t busy() const { return busy_; }
    uint32_t drops() const { return drops_; }
    uint32_t attempts() const { return attempts_; }
    uint64_t bytes() const { return bytes_; }
    bool pending() const { return pending_; }
    bool saturated() const { return saturated_; }
private:
    DiagnosticSnapshot latest_;
    bool pending_;
    uint64_t version_,last_slot_;
    bool attempted_;
    uint32_t overwritten_,busy_,drops_,attempts_;
    uint64_t bytes_;
    bool saturated_;
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
