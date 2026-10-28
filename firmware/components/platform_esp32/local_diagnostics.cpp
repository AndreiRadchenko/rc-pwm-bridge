#include "platform/local_diagnostics.hpp"
#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
namespace platform {
LocalDiagnostics::LocalDiagnostics()
    : queue_(xQueueCreateStatic(1, sizeof(DiagnosticSnapshot), queue_storage_, &queue_control_)),
      task_(nullptr), installed_(false) {}
void LocalDiagnostics::publish(const DiagnosticSnapshot& snapshot) {
    if (queue_) (void)xQueueOverwrite(queue_, &snapshot);
}
bool LocalDiagnostics::transport_ready() {
    if (installed_) return true;
    if (usb_serial_jtag_is_driver_installed()) return false;
    usb_serial_jtag_driver_config_t config{};
    config.tx_buffer_size = 512;
    config.rx_buffer_size = 64;
    installed_ = usb_serial_jtag_driver_install(&config) == ESP_OK;
    return installed_;
}
bool LocalDiagnostics::worker_ready() {
    if (task_) return true;
    if (!queue_ || !installed_) return false;
    task_ = xTaskCreateStatic(run, "diagnostics", sizeof(task_stack_), this,
                              tskIDLE_PRIORITY, task_stack_, &task_control_);
    return task_ != nullptr;
}
std::size_t LocalDiagnostics::write(const char* data, std::size_t length) {
    const int n = usb_serial_jtag_write_bytes(data, length, 0);
    return n > 0 ? static_cast<std::size_t>(n) : 0;
}
void LocalDiagnostics::run(void* arg) {
    auto& self = *static_cast<LocalDiagnostics*>(arg);
    for (;;) {
        DiagnosticSnapshot snapshot{};
        if (xQueuePeek(self.queue_, &snapshot, 0) == pdTRUE)
            (void)self.consumer_.consume(snapshot, self);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
}
