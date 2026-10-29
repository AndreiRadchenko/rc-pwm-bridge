#ifndef PLATFORM_LOCAL_DIAGNOSTICS_HPP
#define PLATFORM_LOCAL_DIAGNOSTICS_HPP
#include "platform/startup_port.hpp"
#include "bridge/rc_capture_trace.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace platform {
class LocalDiagnostics final : public SetupPort, public DiagnosticPort {
public:
    LocalDiagnostics();
    bool transport_ready() override;
    bool worker_ready() override;
    void publish(const DiagnosticSnapshot&) override;
    void publish_capture(const bridge::CaptureSnapshot&);
    void archive(const bridge::AccuracyTrace* trace);
private:
    static void run(void*);
    void step(uint64_t now);
    portMUX_TYPE lock_=portMUX_INITIALIZER_UNLOCKED;
    RcDiagnosticPolicy policy_;
    TaskHandle_t task_;
    bool installed_;
    StaticTask_t task_control_{};
    StackType_t task_stack_[4096 / sizeof(StackType_t)]{};
    const bridge::AccuracyTrace* archive_;
    unsigned cursor_;
};
}
#endif
