#ifndef DANS02_DSTAGE_MULTIQUEUE_H
#define DANS02_DSTAGE_MULTIQUEUE_H

#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/dstage/job.h"

namespace dans {

template <typename T>
class MultiQueue {
 public:
  MultiQueue(unsigned max_priority);
  ~MultiQueue() {}

  // Adds a job_id to all priority queues referenced in prio_list.
  // This function is thread safe.
  void Enqueue(UniqConstJobPtr<T> job_p);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  UniqConstJobPtr<T> Dequeue(Priority prio);

  std::list<UniqConstJobPtr<T>> Purge(JobId job_id);

  // Returns the size of the queue related to Priority prio. There is no
  // guarantee that this value is valid even at the time of the return.
  unsigned Size(Priority prio);

  bool Empty(Priority prio);

  // Returns a nullptr to the next in line for the Priority `prio` queue.
  void ReleaseQueues();

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
  std::vector<std::list<JobId>> _priority_qs;

  // locks the job map meta data
  std::mutex _value_map_mutex;
  std::unordered_map<JobId,
                     std::list<std::pair<UniqConstJobPtr<T>,
                                         typename std::list<JobId>::iterator>>>
      _value_mapper;
};

}  // namespace dans

#endif  // DANS02_DSTAGE_MULTIQUEUE_H
