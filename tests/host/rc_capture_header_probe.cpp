#include "bridge/rc_capture.hpp"
#include "bridge/rc_capture_trace.hpp"
int main() {
  static_assert(sizeof(bridge::AccuracyTrace) <= 6400, "trace bound");
  return 0;
}
