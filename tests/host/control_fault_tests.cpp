#include "bridge/control.hpp"
#include "test_support.hpp"
int main() {
    Checks c; bridge::ControlCore empty;
    empty.update_capture_fault(1);
    CHECK(c, !empty.snapshot().has_valid && empty.snapshot().state == bridge::ControlState::Fault);
    CHECK(c, empty.snapshot().effective_duty_percent == 0);
    bridge::ControlCore core;
    core.update_measurement(1800, 10); core.update_capture_fault(11);
    CHECK(c, core.snapshot().state == bridge::ControlState::Fault && core.snapshot().calculated_duty_percent == 80);
    CHECK(c, core.snapshot().last_valid_time_us == 10 && core.snapshot().effective_duty_percent == 0);
    core.update_measurement(1250, 12);
    CHECK(c, core.snapshot().calculated_duty_percent == 25 && core.snapshot().last_valid_time_us == 12);
    core.update_measurement(2101, 13);
    CHECK(c, core.snapshot().calculated_duty_percent == 25 && core.snapshot().last_valid_time_us == 12);
    CHECK(c, core.snapshot().state == bridge::ControlState::Fault && !core.snapshot().work_permitted);
    return c.failures ? 1 : 0;
}
