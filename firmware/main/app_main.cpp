#include "platform/startup_port.hpp"
#include "platform/safe_output.hpp"
#include "platform/monotonic_clock.hpp"
#include "platform/local_diagnostics.hpp"
#include "board_profile.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern "C" void app_main() {
    static platform::EspMonotonicClock clock;
    static platform::EspBoardProfile profile;
    static platform::EspSafeOutput output(profile);
    static platform::LocalDiagnostics diagnostic;
    static platform::StartupPolicy supervisor(clock, profile, output, diagnostic, diagnostic, BRIDGE_PROFILE_FINGERPRINT);
    for (;;) {
        supervisor.poll();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
