#ifndef DANS02_DSTAGE_MULTIQUEUE_H
#define DANS02_DSTAGE_MULTIQUEUE_H

#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/dstage/basemultiqueue.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
class MultiQueue : public BaseMultiQueue<T> {
 public:
  MultiQueue(unsigned max_priority);
  ~MultiQueue();

  // Adds a job_id to all priority queues referenced in prio_list.
  // This function is thread safe.
  void Enqueue(UniqJobPtr<T> job_p);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  UniqJobPtr<T> Dequeue(Priority prio);

  unsigned Purge(JobId job_id);

  // Returns the size of the queue related to Priority prio. There is no
  // guarantee that this value is valid even at the time of the return.
  unsigned Size(Priority prio);

  bool Empty(Priority prio);

  // Returns a nullptr to the next in line for the Priority `prio` queue.
  void ReleaseQueues();

  void ReleaseOne(Priority prio);

  unsigned MaxPriority();

  std::string DescribeQ(Priority prio);
  std::string DescribeMapper();
  std::string Describe();

 protected:
  const unsigned _max_prio;
  bool _released;

  // purge needs exclusive access to basically everything but all other calls
  // can share.
  std::shared_timed_mutex _purge_shared_mutex;

  // guards the state of each queue ensuring that there is at least a single
  // element in the q
  std::vector<std::mutex> _not_empty_mutexes;

  // lock a specific mutex
  std::vector<std::mutex> _pq_mutexes;
  std::vector<std::list<JobId>> _priority_qs;

  std::vector<bool> _release_one;

  // locks the job map meta data
  std::mutex _value_map_mutex;
  std::unordered_map<
      JobId,
      std::list<std::pair<UniqJobPtr<T>, typename std::list<JobId>::iterator>>>
      _value_mapper;
};

}  // namespace dans

#include "common/dstage/multiqueue.hh"

#endif  // DANS02_DSTAGE_MULTIQUEUE_H
