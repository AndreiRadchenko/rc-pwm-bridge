#include "platform/monotonic_clock.hpp"
#include "esp_timer.h"
namespace platform {
ClockReading EspMonotonicClock::read() {
    return {true, esp_timer_get_time()};
}
}
