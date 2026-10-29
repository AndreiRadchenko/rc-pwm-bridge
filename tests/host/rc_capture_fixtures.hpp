#pragma once
#include "bridge/rc_capture.hpp"
#include "test_support.hpp"
#include <initializer_list>
struct CaptureFixture {
  bridge::CapturePolicy policy;
  uint32_t seq = 0, epoch = 0;
  CaptureFixture(uint64_t now = 0, bool high = false) {
    policy.start(now, high);
  }
  bridge::RxBatch edges(uint64_t t, uint32_t w, int delta = 0) {
    bridge::RxBatch b{};
    b.armed_at_us = t - 1;
    b.epoch = ++epoch;
    b.arm_seq = seq;
    policy.on_edge({t, ++seq, epoch, true}, t);
    policy.on_edge({t + w + delta, ++seq, epoch, false}, t + w + delta);
    b.close_seq = seq;
    b.completed_at_us = t + w + delta + 5000;
    b.rearmed_at_us = b.completed_at_us;
    b.symbol_count = 2;
    b.run_count = 2;
    b.runs[0] = {static_cast<uint16_t>(w), true};
    b.runs[1] = {5000, false};
    return b;
  }
  bool pulse(uint64_t t, uint32_t w, int delta = 0) {
    auto b = edges(t, w, delta);
    return policy.on_batch(b, b.completed_at_us);
  }
};
