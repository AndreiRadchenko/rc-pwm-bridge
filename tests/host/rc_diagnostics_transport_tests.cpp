#include "platform/startup_port.hpp"
#include "test_support.hpp"
#include <cstring>
int main() {
  Checks c;
  // ESP-IDF 5.5.6 rejects RX buffers <= one 64-byte USB packet.
  CHECK(c, platform::DiagnosticTransportBuffers::rx_bytes > 64);
  CHECK(c, platform::DiagnosticTransportBuffers::tx_bytes == 512);
  platform::RcDiagnosticPolicy p;
  platform::DiagnosticSnapshot s{};
  s.sequence = UINT64_MAX;
  s.profile_fingerprint = UINT64_MAX;
  s.capture.measurement.start_us = UINT64_MAX;
  s.capture.measurement.end_us = UINT64_MAX;
  s.capture.measurement.width_us = UINT32_MAX;
  s.capture.measurement.period_us = UINT32_MAX;
  s.capture.counters = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
                        UINT32_MAX, UINT32_MAX, UINT32_MAX, true};
  s.capture.fault_bits = UINT32_MAX;
  s.capture.started_at_us = UINT64_MAX;
  s.capture.max_rearm_us = UINT64_MAX;
  s.capture.max_processing_us = UINT64_MAX;
  s.capture.max_fault_lateness_us = UINT64_MAX;
  s.capture.threshold_at_us = UINT64_MAX;
  s.capture.reported_at_us = UINT64_MAX;
  s.startup.phase = bridge::StartupPhase::ClockFault;
  s.startup.attempts_started = 255;
  s.startup.last_error = bridge::StartupError::ProtocolFault;
  s.pwm_low_api_applied = true;
  char b[385];
  int n = p.format(s, b, sizeof(b));
  CHECK(c, n > 0 && n <= 384 && b[0] == '\n' && b[n - 1] == '\n');
  CHECK(c, p.format(s, b, sizeof(b), true) == 384 && b[383] == '\n');
  CHECK(c, p.format(s, b, 8) == 0);
  CHECK(c, !p.slot(0, false));
  return c.failures ? 1 : 0;
}
