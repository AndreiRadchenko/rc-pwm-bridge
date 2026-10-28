#include "bridge/control.hpp"

namespace bridge {
ControlCore::ControlCore()
    : state_{MeasurementStatus::None, ControlState::Disarmed, 0, 0, 0, false, false},
      last_event_time_us_(0), has_event_time_(false) {}

bool ControlCore::accept_time(uint64_t time_us) {
    if (has_event_time_ && time_us < last_event_time_us_) return false;
    last_event_time_us_ = time_us;
    has_event_time_ = true;
    return true;
}
UpdateStatus ControlCore::update_measurement(uint32_t width_us, uint64_t time_us) {
    if (!accept_time(time_us)) return UpdateStatus::TimeOrderError;
    if (width_us < 900 || width_us > 2100) {
        state_.measurement_status = MeasurementStatus::InvalidWidth;
        return UpdateStatus::Accepted;
    }
    state_.measurement_status = MeasurementStatus::Valid;
    if (width_us <= 1000) state_.calculated_duty_percent = 0;
    else if (width_us >= 2000) state_.calculated_duty_percent = 100;
    else state_.calculated_duty_percent = static_cast<uint8_t>((width_us - 1000 + 5) / 10);
    state_.last_valid_time_us = time_us;
    state_.has_valid = true;
    return UpdateStatus::Accepted;
}
UpdateStatus ControlCore::update_capture_fault(uint64_t time_us) {
    const bool accepted = accept_time(time_us);
    state_.state = ControlState::Fault;
    return accepted ? UpdateStatus::Accepted : UpdateStatus::TimeOrderError;
}
void ControlCore::clear_fault() { state_.state = ControlState::Disarmed; }
ControlSnapshot ControlCore::snapshot() const { return state_; }
}
