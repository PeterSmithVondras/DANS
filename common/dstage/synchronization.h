#ifndef DANS02_DSTAGE_SYNCHRONIZATION_H
#define DANS02_DSTAGE_SYNCHRONIZATION_H

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

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

}  // namespace dans

#endif  // DANS02_DSTAGE_SYNCHRONIZATION_H
