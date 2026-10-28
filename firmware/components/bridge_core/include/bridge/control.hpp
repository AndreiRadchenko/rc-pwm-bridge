#ifndef BRIDGE_CONTROL_HPP
#define BRIDGE_CONTROL_HPP

#include <stdint.h>

namespace bridge {
enum class MeasurementStatus : uint8_t { None, Valid, InvalidWidth };
enum class UpdateStatus : uint8_t { Accepted, TimeOrderError };
enum class ControlState : uint8_t { Disarmed, Fault };
struct ControlSnapshot {
    MeasurementStatus measurement_status;
    ControlState state;
    uint8_t calculated_duty_percent;
    uint8_t effective_duty_percent;
    uint64_t last_valid_time_us;
    bool has_valid;
    bool work_permitted;
};
class ControlCore {
public:
    ControlCore();
    UpdateStatus update_measurement(uint32_t width_us, uint64_t time_us);
    UpdateStatus update_capture_fault(uint64_t time_us);
    void clear_fault();
    ControlSnapshot snapshot() const;
private:
    bool accept_time(uint64_t time_us);
    ControlSnapshot state_;
    uint64_t last_event_time_us_;
    bool has_event_time_;
};
}
#endif
