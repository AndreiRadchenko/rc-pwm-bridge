#ifndef PLATFORM_RC_CAPTURE_HPP
#define PLATFORM_RC_CAPTURE_HPP
#include "bridge/rc_capture.hpp"
#include "bridge/rc_capture_trace.hpp"
#include "driver/rmt_rx.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "platform/local_diagnostics.hpp"
#include "sdkconfig.h"
#include <atomic>
namespace platform {
class RcCapture final : private bridge::CaptureResourcePort {
public:
  explicit RcCapture(LocalDiagnostics &);
  bool start(uint64_t now);

private:
  bool acquire(bridge::CaptureResource) override;
  void release(bridge::CaptureResource) override;
  static void IRAM_ATTR gpio_edge(void *);
  static bool IRAM_ATTR rx_done(rmt_channel_handle_t,
                                const rmt_rx_done_event_data_t *, void *);
  static void tick(void *);
  static void run(void *);
  bool IRAM_ATTR arm_locked(bool from_isr);
  bool IRAM_ATTR pending_edge() const;
  void step();
  void recovery(uint64_t now);
  LocalDiagnostics &diagnostics_;
  bridge::CapturePolicy policy_;
  bridge::CaptureLifecycle resources_;
  portMUX_TYPE lock_ = portMUX_INITIALIZER_UNLOCKED;
  rmt_channel_handle_t channel_ = nullptr;
  esp_timer_handle_t timer_ = nullptr;
  rmt_receive_config_t receive_{};
  rmt_symbol_word_t scratch_[48]{};
  StaticQueue_t edge_control_{}, batch_control_{};
  uint8_t edge_storage_[64 * sizeof(bridge::EdgeEvent)]{};
  uint8_t batch_storage_[8 * sizeof(bridge::RxBatch)]{};
  QueueHandle_t edges_, batches_;
  TaskHandle_t task_ = nullptr;
  StaticTask_t task_control_{};
  StackType_t stack_[8192 / sizeof(StackType_t)]{};
  bridge::LineObservation line_{};
  uint64_t armed_at_ = 0, started_at_ = 0, retry_at_ = 0;
  uint32_t epoch_ = 0, arm_seq_ = 0, pending_fault_ = 0, pending_losses_ = 0;
  bool gpio_service_owned_ = false, gpio_added_ = false, enabled_ = false;
  bool initialized_ = false, rx_active_ = false, recovery_requested_ = false,
       stopped_ = false;
  std::atomic<bool> running_{false};
#ifdef CONFIG_BRIDGE_RC03_ACCURACY_TRACE
  bridge::AccuracyTrace trace_;
#endif
};
} // namespace platform
#endif
