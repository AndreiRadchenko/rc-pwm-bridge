#ifndef PLATFORM_SAFE_OUTPUT_HPP
#define PLATFORM_SAFE_OUTPUT_HPP
#include "platform/startup_port.hpp"
namespace platform {
class EspBoardProfile final : public ProfilePort {
public:
    bool matches() override;
};
class EspSafeOutput final : public OutputPort {
public:
    explicit EspSafeOutput(ProfilePort& profile) : profile_(profile) {}
    OutputApplyResult apply_low() override;
private:
    ProfilePort& profile_;
};
}
#endif
