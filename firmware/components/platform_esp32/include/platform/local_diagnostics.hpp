#ifndef PLATFORM_LOCAL_DIAGNOSTICS_HPP
#define PLATFORM_LOCAL_DIAGNOSTICS_HPP
#include "platform/startup_port.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
namespace platform {
class LocalDiagnostics final : public SetupPort, public DiagnosticPort, private TransportPort {
public:
    LocalDiagnostics();
    bool transport_ready() override;
    bool worker_ready() override;
    void publish(const DiagnosticSnapshot&) override;
private:
    std::size_t write(const char*, std::size_t) override;
    static void run(void*);
    StaticQueue_t queue_control_{};
    alignas(DiagnosticSnapshot) uint8_t queue_storage_[sizeof(DiagnosticSnapshot)]{};
    QueueHandle_t queue_;
    StaticTask_t task_control_{};
    StackType_t task_stack_[2048]{};
    TaskHandle_t task_;
    bool installed_;
    DiagnosticConsumer consumer_;
};
}
#endif
