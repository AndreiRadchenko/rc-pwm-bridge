#ifndef PLATFORM_MONOTONIC_CLOCK_HPP
#define PLATFORM_MONOTONIC_CLOCK_HPP
#include "platform/startup_port.hpp"
namespace platform {
class EspMonotonicClock final : public ClockPort {
public:
    ClockReading read() override;
};
}
#endif
