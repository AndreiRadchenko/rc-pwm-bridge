#include "bridge/control.hpp"
#include "test_support.hpp"
struct FakeClock { uint64_t us; explicit FakeClock(uint64_t start):us(start) {} };
int main() {
    Checks c; bridge::ControlCore core; FakeClock clock(20);
    CHECK(c, core.update_measurement(1800, clock.us) == bridge::UpdateStatus::Accepted);
    CHECK(c, core.update_measurement(1250, clock.us) == bridge::UpdateStatus::Accepted);
    clock.us = 21; core.update_measurement(2101, clock.us);
    CHECK(c, core.snapshot().last_valid_time_us == 20);
    clock.us = 20;
    CHECK(c, core.update_measurement(2000, clock.us) == bridge::UpdateStatus::TimeOrderError);
    CHECK(c, core.snapshot().calculated_duty_percent == 25 && core.snapshot().last_valid_time_us == 20 && core.snapshot().measurement_status == bridge::MeasurementStatus::InvalidWidth);
    CHECK(c, core.update_capture_fault(clock.us) == bridge::UpdateStatus::TimeOrderError);
    CHECK(c, core.snapshot().state == bridge::ControlState::Fault && core.snapshot().effective_duty_percent == 0 && core.snapshot().last_valid_time_us == 20);
    clock.us = 1000000000ULL; core.update_measurement(1500, clock.us);
    CHECK(c, core.snapshot().state == bridge::ControlState::Fault && core.snapshot().calculated_duty_percent == 50);
    CHECK(c, core.snapshot().last_valid_time_us == clock.us && !core.snapshot().work_permitted);
    return c.failures ? 1 : 0;
}
