#include "bridge/control.hpp"
#include "test_support.hpp"
#include <stdint.h>
int main() {
    Checks c; bridge::ControlCore core;
    core.update_measurement(2101, 1);
    CHECK(c, !core.snapshot().has_valid && core.snapshot().measurement_status == bridge::MeasurementStatus::InvalidWidth);
    const uint32_t vals[] = {0,899,900,2100,2101,UINT32_MAX};
    const bool valid[] = {false,false,true,true,false,false};
    for (unsigned i=0;i<6;++i) {
        core.update_measurement(vals[i], i+2);
        CHECK(c, (core.snapshot().measurement_status == bridge::MeasurementStatus::Valid) == valid[i]);
    }
    core.update_measurement(1800, 10);
    core.update_measurement(UINT32_MAX, 11);
    CHECK(c, core.snapshot().measurement_status == bridge::MeasurementStatus::InvalidWidth);
    CHECK(c, core.snapshot().calculated_duty_percent == 80 && core.snapshot().last_valid_time_us == 10);
    core.update_measurement(1250, 12);
    CHECK(c, core.snapshot().calculated_duty_percent == 25 && core.snapshot().last_valid_time_us == 12);
    CHECK(c, core.snapshot().state == bridge::ControlState::Disarmed && !core.snapshot().work_permitted);
    return c.failures ? 1 : 0;
}
