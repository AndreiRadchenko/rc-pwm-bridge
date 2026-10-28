#include "bridge/control.hpp"
#include "test_support.hpp"
int main() {
    Checks c; bridge::ControlCore core;
    CHECK(c, !core.snapshot().has_valid && !core.snapshot().work_permitted && core.snapshot().effective_duty_percent == 0);
    core.update_measurement(2000, 1);
    CHECK(c, core.snapshot().calculated_duty_percent == 100 && core.snapshot().effective_duty_percent == 0 && !core.snapshot().work_permitted);
    core.update_capture_fault(2);
    CHECK(c, core.snapshot().state == bridge::ControlState::Fault && core.snapshot().effective_duty_percent == 0);
    core.clear_fault();
    CHECK(c, core.snapshot().state == bridge::ControlState::Disarmed && core.snapshot().effective_duty_percent == 0 && !core.snapshot().work_permitted);
    core.update_measurement(2000, 3);
    CHECK(c, core.snapshot().effective_duty_percent == 0 && !core.snapshot().work_permitted);
    return c.failures ? 1 : 0;
}
