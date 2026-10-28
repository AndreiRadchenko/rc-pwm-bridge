#include "platform/startup_port.hpp"
#include <cstdio>

namespace platform {
bool checked_clock(ClockReading reading, bridge::MonotonicUs& result) {
    if (!reading.valid || reading.raw_us < 0) return false;
    result = static_cast<bridge::MonotonicUs>(reading.raw_us);
    return true;
}
StartupPolicy::StartupPolicy(ClockPort& c, ProfilePort& p, OutputPort& o, SetupPort& s, DiagnosticPort& d, uint64_t fp)
    : clock_(c), profile_(p), output_(o), setup_(s), diagnostic_(d), supervisor_(), last_{}, published_(false), low_(false) {
    last_.profile_fingerprint = fp;
    publish_if_changed(true);
}
void StartupPolicy::publish_if_changed(bool force) {
    const auto current = supervisor_.snapshot();
    const auto& prev = last_.startup;
    if (force || !published_ || current.phase != prev.phase || current.attempts_started != prev.attempts_started ||
        current.deadline_valid != prev.deadline_valid || (current.deadline_valid && current.next_attempt_us != prev.next_attempt_us) ||
        current.last_error != prev.last_error || low_ != last_.pwm_low_api_applied) {
        ++last_.sequence;
        last_.startup = current;
        last_.pwm_low_api_applied = low_;
        diagnostic_.publish(last_);
        published_ = true;
    }
}
void StartupPolicy::poll() {
    bridge::MonotonicUs now;
    if (!checked_clock(clock_.read(), now)) { supervisor_.clock_fault(); publish_if_changed(); return; }
    if (!supervisor_.tick(now)) { publish_if_changed(); return; }
    publish_if_changed();
    bridge::InitOutcome outcome{false, bridge::StartupError::Profile};
    if (profile_.matches()) {
        low_ = output_.apply_low().pwm_low_api_applied;
        outcome.error = bridge::StartupError::Output;
        if (low_) {
            outcome.error = bridge::StartupError::Transport;
            if (setup_.transport_ready()) {
                outcome.error = bridge::StartupError::Worker;
                if (setup_.worker_ready()) outcome = {true, bridge::StartupError::None};
            }
        }
    }
    if (!checked_clock(clock_.read(), now)) supervisor_.clock_fault();
    else supervisor_.result(now, outcome);
    publish_if_changed();
}
bool DiagnosticConsumer::consume(const DiagnosticSnapshot& s, TransportPort& transport) {
    if (attempted_ && sequence_ == s.sequence) return false;
    attempted_ = true; sequence_ = s.sequence;
    char buf[256];
    int n = std::snprintf(buf, sizeof(buf), "seq=%llu phase=%u attempts=%u error=%u low=%u permitted=%u\n",
        static_cast<unsigned long long>(s.sequence), static_cast<unsigned>(s.startup.phase),
        static_cast<unsigned>(s.startup.attempts_started), static_cast<unsigned>(s.startup.last_error),
        static_cast<unsigned>(s.pwm_low_api_applied), static_cast<unsigned>(s.startup.work_permitted));
    if (n > 0 && static_cast<std::size_t>(n) < sizeof(buf)) transport.write(buf, static_cast<std::size_t>(n));
    return true;
}
}
