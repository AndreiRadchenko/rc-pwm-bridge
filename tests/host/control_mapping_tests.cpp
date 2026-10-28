#include "bridge/control.hpp"
#include "test_support.hpp"
#include <stdint.h>
int main() {
    Checks c; bridge::ControlCore core;
    const uint32_t widths[] = {1000,1250,1400,1500,1800,2000,900,999,1001,1999,2001,2100};
    const uint8_t expected[] = {0,25,40,50,80,100,0,0,0,100,100,100};
    uint64_t time = 0;
    for (unsigned pass = 0; pass < 2; ++pass)
        for (unsigned i = 0; i < sizeof(widths)/sizeof(widths[0]); ++i) {
            CHECK(c, core.update_measurement(widths[i], time++) == bridge::UpdateStatus::Accepted);
            CHECK(c, core.snapshot().calculated_duty_percent == expected[i]);
            CHECK(c, core.snapshot().has_valid && core.snapshot().effective_duty_percent == 0);
        }
    for (uint32_t k = 0; k < 100; ++k) {
        uint32_t boundary = 1005 + 10*k;
        for (int offset = -1; offset <= 1; ++offset) {
            uint32_t w = static_cast<uint32_t>(static_cast<int>(boundary) + offset);
            core.update_measurement(w, time++);
            CHECK(c, core.snapshot().calculated_duty_percent == (offset < 0 ? k : k+1));
        }
    }
    return c.failures ? 1 : 0;
}
