#include "common/dstage/job_state_tracker.h"

#include "glog/logging.h"

namespace dans {

PurgeState::PurgeState()
    : _purged(false) {}

bool PurgeState::IsPurged() {
  std::shared_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  bool purged = _purged;
  return purged;
}

bool PurgeState::SetPurged() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  bool set = !_purged;
  _purged = true;
  return set;
}

Counter::Counter(int initial_value) : _count(initial_value) {}

void Counter::Increment() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  _count++;
}

void Counter::Decrement() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  _count--;
}

int Counter::Count() {
  std::shared_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  int count = _count;
  return count;
}

}  // namespace dans
