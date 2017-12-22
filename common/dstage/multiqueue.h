#ifndef DANS02_DSTAGE_MULTIQUEUE_H
#define DANS02_DSTAGE_MULTIQUEUE_H

#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {

template <typename Key, typename Value>
class MultiQueue {
 public:
  MultiQueue(unsigned max_priority);
  ~MultiQueue() {}

  // Adds a job_id to all priority queues referenced in prio_list.
  // This function is thread safe.
  void Enqueue(std::pair<Key, Value> key_value, Priority prio);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  Value Dequeue(Priority prio);

  std::list<Priority> Purge(Key key);

  // Returns the size of the queue related to Priority prio. There is no
  // guarantee that this value is valid even at the time of the return.
  unsigned Size(Priority prio);

  bool Empty(Priority prio);

 protected:
  const unsigned _max_prio;

  // purge needs exclusive access to basically everything but all other calls
  // can share.
  std::shared_timed_mutex _purge_shared_mutex;

  // guards the state of each queue ensuring that there is at least a single
  // element in the q
  std::vector<std::mutex> _not_empty_mutexes;

  // lock a specific mutex
  std::vector<std::mutex> _pq_mutexes;
  std::vector<std::list<std::pair<Key, Value>>> _priority_qs;

  // locks the job map meta data
  std::mutex _value_map_mutex;
  std::unordered_map<
      Key, std::list<std::pair<
               Priority, typename std::list<std::pair<Key, Value>>::iterator>>>
      _value_mapper;
};

}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_MULTIQUEUE_H
