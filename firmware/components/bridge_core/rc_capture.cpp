#include "bridge/rc_capture.hpp"
#include <limits.h>
namespace bridge {
void increment(uint32_t &v, bool &s) {
  if (v == UINT32_MAX)
    s = true;
  else
    ++v;
}
CapturePolicy::CapturePolicy()
    : edges_{}, count_(0), core_(), state_{}, last_time_(0), barrier_(0),
      origin_(0), level_since_(0), guard_start_(0), last_seq_(0),
      guard_epoch_(0), has_time_(false), has_seq_(false), has_origin_(false),
      high_(false), episode_reported_(false), guard_active_(false) {}
void CapturePolicy::start(uint64_t now, bool high) {
  last_time_ = now;
  has_time_ = true;
  level_since_ = now;
  high_ = high;
  has_seq_ = true;
  last_seq_ = 0;
  state_.started_at_us = now;
  if (high) state_.measurement.status = CaptureStatus::Incomplete;
}
bool CapturePolicy::clock_ok(uint64_t now) {
  if (has_time_ && now < last_time_) {
    fault(ClockError, now);
    return false;
  }
  last_time_ = now;
  has_time_ = true;
  return true;
}
void CapturePolicy::fault(uint32_t bits, uint64_t now, uint32_t dropped) {
  // A regressed clock must still latch the core. Use the last trusted event
  // time for ordering; never change the captured sample or its last-valid time.
  if (has_time_ && now < last_time_) {
    bits |= ClockError;
    now = last_time_;
  }
  last_time_ = now;
  has_time_ = true;
  if (!state_.counters.faults) state_.first_fault_at_us = now;
  state_.last_fault_at_us = now;
  if (dropped > UINT32_MAX - state_.counters.dropped) {
    state_.counters.dropped = UINT32_MAX;
    state_.counters.saturated = true;
  } else
    state_.counters.dropped += dropped;
  state_.fault_bits |= bits;
  state_.recovery_needed = true;
  increment(state_.counters.faults, state_.counters.saturated);
  state_.measurement.status = CaptureStatus::Error;
  (void)core_.update_capture_fault(now);
  if (now > barrier_)
    barrier_ = now;
  count_ = 0;
  has_origin_ = false;
  has_seq_ = false;
  guard_active_ = false;
}
void CapturePolicy::episode(uint64_t end, uint64_t now) {
  if (end < level_since_) {
    fault(ClockError, now);
    return;
  }
  if (!episode_reported_ && end - level_since_ >= 100000) {
    episode_reported_ = true;
    state_.threshold_at_us = level_since_ + 100000;
    state_.reported_at_us = now;
    uint64_t late =
        now >= state_.threshold_at_us ? now - state_.threshold_at_us : 0;
    if (late > state_.max_fault_lateness_us)
      state_.max_fault_lateness_us = late;
    fault((high_ ? StuckHigh : StuckLow) |
              (late > 10000 ? uint32_t(DeadlineMissed) : 0u),
          now);
  }
}
void CapturePolicy::on_edge(const EdgeEvent &e, uint64_t now) {
  if (!clock_ok(now))
    return;
  if (e.time_us > now) {
    fault(ClockError, now);
    return;
  }
  if (e.time_us < barrier_) {
    increment(state_.counters.dropped, state_.counters.saturated);
    return;
  }
  if ((has_seq_ && e.seq == last_seq_) || e.high == high_)
    increment(state_.counters.duplicate_edges, state_.counters.saturated);
  if ((has_seq_ && e.seq != uint32_t(last_seq_ + 1)) || e.high == high_ ||
      e.time_us < level_since_) {
    fault(ShapeError, now);
    return;
  }
  episode(e.time_us, now);
  high_ = e.high;
  level_since_ = e.time_us;
  episode_reported_ = false;
  last_seq_ = e.seq;
  has_seq_ = true;
  if (count_ == 64) {
    increment(state_.counters.dropped, state_.counters.saturated);
    fault(QueueLoss, now);
    return;
  }
  SavedEdge saved{e, 0, false};
  if (e.high) {
    if (has_origin_ && e.time_us >= origin_ &&
        e.time_us - origin_ <= UINT32_MAX) {
      saved.period = uint32_t(e.time_us - origin_);
      saved.known = true;
    }
    origin_ = e.time_us;
    has_origin_ = true;
    if (!guard_active_ || guard_epoch_ != e.epoch) {
      guard_start_ = e.time_us;
      guard_epoch_ = e.epoch;
      guard_active_ = true;
    }
  }
  edges_[count_++] = saved;
}
bool CapturePolicy::on_batch(const RxBatch &b, uint64_t now) {
  if (!clock_ok(now))
    return false;
  increment(state_.counters.batches, state_.counters.saturated);
  if (guard_active_ && guard_epoch_ == b.epoch)
    guard_active_ = false;
  if (b.armed_at_us < barrier_) {
    increment(state_.counters.dropped, state_.counters.saturated);
    return false;
  }
  if (b.completed_at_us > now || b.armed_at_us > b.completed_at_us ||
      (b.rearmed_at_us && (b.rearmed_at_us < b.completed_at_us || b.rearmed_at_us > now))) {
    fault(ClockError, now);
    return false;
  }
  uint64_t processing = now - b.completed_at_us;
  if (processing > state_.max_processing_us)
    state_.max_processing_us = processing;
  if (processing > 10000) {
    fault(ProcessingLate, now);
    return false;
  }
  if (b.symbol_count >= 48 || b.run_count > 96) {
    fault(Saturation, now);
    return false;
  }
  SavedEdge pair[2]{};
  size_t n = 0, keep = 0;
  uint32_t span = b.close_seq - b.arm_seq;
  if (span >= 0x80000000u) {
    fault(ShapeError, now);
    return false;
  }
  for (size_t i = 0; i < count_; ++i) {
    uint32_t d = edges_[i].edge.seq - b.arm_seq;
    if (d && d <= span && edges_[i].edge.epoch == b.epoch) {
      if (n < 2)
        pair[n] = edges_[i];
      ++n;
    } else if (uint32_t(edges_[i].edge.seq - b.close_seq) < 0x80000000u &&
               edges_[i].edge.seq != b.close_seq)
      edges_[keep++] = edges_[i];
  }
  count_ = keep;
  if (n == 1 && pair[0].edge.high && b.final_high) {
    state_.measurement = {pair[0].edge.time_us,
                          0,
                          0,
                          pair[0].period,
                          pair[0].edge.seq,
                          b.epoch,
                          CaptureStatus::Incomplete,
                          false,
                          pair[0].known};
    increment(state_.counters.candidates, state_.counters.saturated);
    increment(state_.counters.invalid, state_.counters.saturated);
    return true;
  }
  if (n != 2 || !pair[0].edge.high || pair[1].edge.high || b.final_high ||
      pair[1].edge.time_us <= pair[0].edge.time_us) {
    fault(ShapeError, now);
    return false;
  }
  if (pair[0].edge.time_us < b.armed_at_us || pair[1].edge.time_us > b.completed_at_us) {
    fault(ClockError,now);return false;
  }
  uint32_t width = 0;
  unsigned highs = 0;
  bool ended = false, seen_low_after = false;
  for (size_t i = 0; i < b.run_count; ++i) {
    Run v = b.runs[i];
    if (!v.duration_us) {
      ended = true;
      continue;
    }
    if (ended || v.duration_us > 32767) {
      fault(ShapeError, now);
      return false;
    }
    if (v.high) {
      if (seen_low_after) {
        fault(ShapeError, now);
        return false;
      }
      ++highs;
      width = v.duration_us;
    } else if (highs)
      seen_low_after = true;
  }
  uint64_t gpio_width = pair[1].edge.time_us - pair[0].edge.time_us;
  uint64_t diff = gpio_width > width ? gpio_width - width : width - gpio_width;
  if (highs != 1 || diff > 50 || (!ended && !seen_low_after)) {
    fault(ShapeError, now);
    return false;
  }
  if (b.rearmed_at_us) {
    uint64_t stop = pair[1].edge.time_us + 5000;
    if (b.rearmed_at_us >= stop) {
      uint64_t gap = b.rearmed_at_us - stop;
      if (gap > state_.max_rearm_us)
        state_.max_rearm_us = gap;
      if (gap > 1000) {
        fault(RearmLate, now);
        return false;
      }
    }
  }
  CaptureStatus status = CaptureStatus::Valid;
  if (width < 900 || width > 2100)
    status = CaptureStatus::InvalidWidth;
  else if (!pair[0].known)
    status = CaptureStatus::PeriodUnknown;
  else if (pair[0].period < 18000 || pair[0].period > 22000)
    status = CaptureStatus::InvalidPeriod;
  state_.measurement = {pair[0].edge.time_us,
                        pair[1].edge.time_us,
                        width,
                        pair[0].period,
                        pair[0].edge.seq,
                        b.epoch,
                        status,
                        true,
                        pair[0].known};
  increment(state_.counters.candidates, state_.counters.saturated);
  if (status == CaptureStatus::Valid) {
    if (core_.update_measurement(width, pair[1].edge.time_us) !=
        UpdateStatus::Accepted) {
      fault(ClockError, now);
      return false;
    }
    increment(state_.counters.valid, state_.counters.saturated);
  } else
    increment(state_.counters.invalid, state_.counters.saturated);
  return true;
}
void CapturePolicy::poll(const LineObservation &l, uint64_t now) {
  if (!clock_ok(now))
    return;
  if (l.stable && l.continuous) {
    if (l.changed_us > now || l.started_us > now) {
      fault(ClockError, now);
      return;
    }
    // Edge queue is drained before poll. A newer unseen edge cannot prove
    // stuck.
    if (l.seq == last_seq_) {
      if (l.high != high_) {
        high_ = l.high;
        level_since_ = l.changed_us;
        episode_reported_ = false;
      }
      episode(now, now);
    }
  }
  if (guard_active_ && now >= guard_start_ && now - guard_start_ >= 12000)
    fault(GuardExpired, now);
}
void CapturePolicy::processing_complete(uint64_t completed_at_us,
                                        uint64_t now) {
  if (!clock_ok(now))
    return;
  if (now < completed_at_us) {
    fault(ClockError, now);
    return;
  }
  const uint64_t elapsed = now - completed_at_us;
  if (elapsed > state_.max_processing_us)
    state_.max_processing_us = elapsed;
  if (elapsed > 10000)
    fault(ProcessingLate, now);
}
void CapturePolicy::recovered(uint64_t now, bool high) {
  count_ = 0;
  has_seq_ = true;
  has_origin_ = false;
  guard_active_ = false;
  if (high != high_)
    episode_reported_ = false;
  high_ = high;
  level_since_ = now;
  state_.recovery_needed = false;
  if (high) {
    state_.measurement = {};
    state_.measurement.status = CaptureStatus::Incomplete;
  }
}
void CapturePolicy::recovered(uint64_t now, bool high, uint32_t sequence) {
  recovered(now, high);
  last_seq_ = sequence;
}
CaptureSnapshot CapturePolicy::snapshot() const {
  CaptureSnapshot s = state_;
  s.control = core_.snapshot();
  s.phase = s.recovery_needed ? CaptureState::Resync :
      high_ ? (guard_active_ ? CaptureState::Receiving : CaptureState::WaitingLow) : CaptureState::WaitingRise;
  return s;
}
bool CaptureLifecycle::setup(CaptureResourcePort &p, uint64_t now) {
  if (owned_ == 15)
    return true;
  if (now < retry_at_)
    return false;
  for (unsigned i = 0; i < 4; ++i) {
    if (!p.acquire(static_cast<CaptureResource>(i))) {
      cleanup(p);
      retry_at_ = now + 100000;
      return false;
    }
    owned_ |= uint8_t(1u << i);
  }
  return true;
}
void CaptureLifecycle::cleanup(CaptureResourcePort &p) {
  for (int i = 3; i >= 0; --i)
    if (owned_ & (1u << i)) {
      p.release(static_cast<CaptureResource>(i));
      owned_ &= uint8_t(~(1u << i));
    }
}
} // namespace bridge
