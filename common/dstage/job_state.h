#ifndef DANS02_DSTAGE_JOB_STATE_H
#define DANS02_DSTAGE_JOB_STATE_H

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/dstage/job.h"
#include "glog/logging.h"

namespace dans {

// Thread safe object used for tracking if a job is purged.
class PurgeState {
 public:
  PurgeState();
  // Returns whether the this job is marked as purged. There is no guarantee
  // that this is accurate even at the time of return.
  bool IsPurged();
  // If the job is not already purged, this will set it as purged and return
  // true. If another thread sets the the job purged, then it will return false.
  bool SetPurged();

 private:
  // guards state of whether job has been purged.
  std::shared_timed_mutex _state_shared_mutex;
  bool _purged;
};

// Thread safe counter.
class Counter {
 public:
  Counter(int initial_value);
  void Increment();
  void Decrement();
  int Count();

 private:
  // guards state of whether job has been purged.
  std::shared_timed_mutex _state_shared_mutex;
  int _count;
};

template <typename T>
class JobState : public PurgeState {
 public:
  JobState(unsigned size) : _priority_based_state(size) {}
  // Add(Priority prio, std::unique_ptr<T>);
  // T* View(Priority prio);
  // std::unique_ptr<T> Remove(Priority prio);
  int Count() {
    int count = 0;
    for (int i = 0; i < _priority_based_state.size(); i++) {
      if (_priority_based_state[i] != nullptr) count++;
    }
    return count;
  }

  std::vector<std::unique_ptr<T>> _priority_based_state;
};

template <typename T>
class JobStateMap {
 public:
  JobStateMap() {}

  // Note that calling Get will return nullptr if not found.
  std::shared_ptr<T> Get(JobId job_id) {
    std::shared_lock<std::shared_timed_mutex> shared_lock(_state_shared_mutex);
    auto iter = _job_state_map.find(job_id);
    return (iter == _job_state_map.end()) ? nullptr : iter->second;
  }

  // Either returns your shared_ptr or returns the pointer that was already
  // their.
  std::shared_ptr<T> Add(JobId job_id, std::shared_ptr<T> ptr) {
    if (ptr == nullptr) {
      LOG(WARNING) << "Attempted to add a nullptr for job_id=" << job_id;
      return nullptr;
    }
    std::unique_lock<std::shared_timed_mutex> unique_lock(_state_shared_mutex);
    auto pair = _job_state_map.insert({job_id, ptr});
    return pair.first->second;
  }

  void remove(JobId job_id) {
    std::unique_lock<std::shared_timed_mutex> unique_lock(_state_shared_mutex);
    auto pair = _job_state_map.erase(job_id);
  }

 private:
  // guards state of whether job has been purged.
  std::shared_timed_mutex _state_shared_mutex;
  std::unordered_map<JobId, std::shared_ptr<T>> _job_state_map;
};

}  // namespace dans

#endif  // DANS02_DSTAGE_JOB_STATE_H
