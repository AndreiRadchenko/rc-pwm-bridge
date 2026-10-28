#include "bridge/startup.hpp"
#include <stdint.h>

namespace bridge {
StartupSupervisor::StartupSupervisor() : state_{0, 0, StartupPhase::AwaitingAttempt, StartupError::None, 0, false, false, false}, has_time_(false) {}
bool StartupSupervisor::accept_time(MonotonicUs now) {
    if (has_time_ && now < state_.now_us) { clock_fault(); return false; }
    state_.now_us = now;
    state_.now_valid = true;
    has_time_ = true;
    return true;
}
void StartupSupervisor::clock_fault() {
    state_.now_valid = false;
    state_.phase = StartupPhase::ClockFault;
    state_.last_error = StartupError::Clock;
    state_.deadline_valid = false;
    state_.work_permitted = false;
}
bool StartupSupervisor::tick(MonotonicUs now) {
    if (state_.phase == StartupPhase::ClockFault) return false;
    if (!accept_time(now)) return false;
    if (state_.phase == StartupPhase::AwaitingAttempt ||
        (state_.phase == StartupPhase::RetryWait && now >= state_.next_attempt_us)) {
        ++state_.attempts_started;
        state_.phase = StartupPhase::Attempting;
        state_.deadline_valid = false;
        return true;
    }
    return false;
}
void StartupSupervisor::result(MonotonicUs now, InitOutcome outcome) {
    if (state_.phase == StartupPhase::ClockFault) return;
    if (!accept_time(now)) return;
    if (state_.phase != StartupPhase::Attempting) {
        state_.phase = StartupPhase::Exhausted;
        state_.last_error = StartupError::ProtocolFault;
        state_.deadline_valid = false;
        return;
    }
    state_.last_error = outcome.success ? StartupError::None : outcome.error;
    if (outcome.success) { state_.phase = StartupPhase::ReadyDisabled; return; }
    if (state_.attempts_started >= RetryPolicy::max_attempts) {
        state_.phase = StartupPhase::Exhausted;
        return;
    }
    if (UINT64_MAX - now < RetryPolicy::retry_delay_us) { clock_fault(); return; }
    state_.next_attempt_us = now + RetryPolicy::retry_delay_us;
    state_.deadline_valid = true;
    state_.phase = StartupPhase::RetryWait;
}
StartupSnapshot StartupSupervisor::snapshot() const { return state_; }
}
