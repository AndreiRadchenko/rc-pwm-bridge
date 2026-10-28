#ifndef BRIDGE_STARTUP_HPP
#define BRIDGE_STARTUP_HPP

#include <stdint.h>

namespace bridge {
typedef uint64_t MonotonicUs;
enum class StartupPhase : uint8_t { AwaitingAttempt, Attempting, RetryWait, ReadyDisabled, Exhausted, ClockFault };
enum class StartupError : uint8_t { None, Profile, Output, Transport, Worker, Clock, ProtocolFault };
struct RetryPolicy {
    static const uint8_t max_attempts = 5;
    static const MonotonicUs retry_delay_us = 1000000;
};
struct InitOutcome { bool success; StartupError error; };
struct StartupSnapshot {
    MonotonicUs now_us;
    MonotonicUs next_attempt_us;
    StartupPhase phase;
    StartupError last_error;
    uint8_t attempts_started;
    bool now_valid;
    bool deadline_valid;
    bool work_permitted;
};
class StartupSupervisor {
public:
    StartupSupervisor();
    bool tick(MonotonicUs now);
    void result(MonotonicUs now, InitOutcome outcome);
    void clock_fault();
    StartupSnapshot snapshot() const;
private:
    bool accept_time(MonotonicUs now);
    StartupSnapshot state_;
    bool has_time_;
};
}
#endif
