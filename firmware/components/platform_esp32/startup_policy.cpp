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

namespace platform {
void RcDiagnosticPolicy::publish(const DiagnosticSnapshot& s) {
    if(pending_) bridge::increment(overwritten_,saturated_);
    latest_=s; latest_.sequence=++version_; pending_=true;
}
void RcDiagnosticPolicy::ack(uint64_t v) { if(v==latest_.sequence) pending_=false; }
bool RcDiagnosticPolicy::slot(uint64_t now,bool drained) {
    if(attempted_ && (now<last_slot_ || now-last_slot_<100000)) return false;
    attempted_=true; last_slot_=now;
    bridge::increment(attempts_,saturated_);
    if(!drained) { bridge::increment(busy_,saturated_);return false; }
    return true;
}
void RcDiagnosticPolicy::written(size_t n,size_t expected) {
    if(n>UINT64_MAX-bytes_) {bytes_=UINT64_MAX;saturated_=true;}
    else bytes_+=n;
    if(n!=expected) bridge::increment(drops_,saturated_);
}
int RcDiagnosticPolicy::format(const DiagnosticSnapshot& s,char* b,size_t cap,bool padded) const {
    auto c=s.capture;
    int n=std::snprintf(b,cap,
        "\nrc03 q=%llx id=%llx sid=%llx w=%u s=%llx e=%llx p=%u v=%u c=%u d=%u o=0 a=0 f=%x n=%x,%x,%x,%x,%x,%x k=%x,%x,%x,%x,%llx m=%llx,%llx,%llx ft=%llx,%llx b=%x,%x,%x,%x sat=%u\n",
        (unsigned long long)s.sequence,(unsigned long long)s.profile_fingerprint,
        (unsigned long long)c.started_at_us,
        unsigned(c.measurement.width_us),(unsigned long long)c.measurement.start_us,
        (unsigned long long)c.measurement.end_us,unsigned(c.measurement.period_us),
        unsigned(c.measurement.status),unsigned(c.phase),unsigned(c.control.calculated_duty_percent),
        unsigned(c.fault_bits),unsigned(c.counters.candidates),unsigned(c.counters.valid),
        unsigned(c.counters.invalid),unsigned(c.counters.dropped),unsigned(c.counters.faults),unsigned(c.counters.duplicate_edges),
        unsigned(overwritten_),unsigned(busy_),unsigned(drops_),unsigned(attempts_),(unsigned long long)bytes_,
        (unsigned long long)c.max_rearm_us,(unsigned long long)c.max_processing_us,
        (unsigned long long)c.max_fault_lateness_us,(unsigned long long)c.threshold_at_us,
        (unsigned long long)c.reported_at_us,unsigned(s.startup.phase),unsigned(s.startup.attempts_started),
        unsigned(s.startup.last_error),unsigned(s.pwm_low_api_applied),unsigned(saturated_||c.counters.saturated));
    if(n<=0 || size_t(n)>=cap) return 0;
    if(padded) {
        if(cap<385) return 0;
        for(int i=n-1;i<383;++i)b[i]=' ';
        b[383]='\n';b[384]=0;return 384;
    }
    return n;
}
}
