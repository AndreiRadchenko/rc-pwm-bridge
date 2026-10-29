#include "platform/startup_port.hpp"
#include "test_support.hpp"
int main() {
  Checks c;
  platform::RcDiagnosticPolicy p;
  platform::DiagnosticSnapshot s{};
  p.publish(s);
  auto v = p.copy().sequence;
  p.publish(s);
  CHECK(c, p.overwritten() == 1);
  p.ack(v);
  CHECK(c, p.pending());
  p.ack(p.copy().sequence);
  CHECK(c, !p.pending());
  CHECK(c, p.slot(0, true));
  CHECK(c, !p.slot(99999, true));
  CHECK(c, !p.slot(100000, false) && p.busy() == 1);
  CHECK(c, !p.slot(199999, true));
  CHECK(c, p.slot(200000, true));
  p.written(0, 10);
  p.written(5, 10);
  CHECK(c, p.drops() == 2 && p.attempts() == 3 && p.bytes() == 5);
  bool sat = false;
  uint32_t x = UINT32_MAX;
  bridge::increment(x, sat);
  CHECK(c, sat && x == UINT32_MAX);
  return c.failures ? 1 : 0;
}
