#ifndef PLATFORM_SAFE_OUTPUT_HPP
#define PLATFORM_SAFE_OUTPUT_HPP
#include "platform/startup_port.hpp"
namespace platform {
class EspBoardProfile final : public ProfilePort {
public:
    bool matches() override;
};
}
#endif
