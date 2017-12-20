#include "common/dstage/multiqueue.h"

#include <cassert>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace {
unsigned kMinimumQueueSize = 0;
}

namespace duplicate_aware_scheduling {

MultiQueue::MultiQueue(unsigned number_of_qs)
    : _max_prio(number_of_qs),
      _not_empty_mutexes(_max_prio),
      _pq_mutexes(_max_prio),
      _priority_qs(_max_prio) {
  for (unsigned i = 0; i < _max_prio; i++) {
    // Queues start empty and therefore "not empty" is locked.
    _not_empty_mutexes[i].lock();
  }
}

void MultiQueue::Enqueue(JobId job_id, std::vector<Priority> prio_list) {
  // Ensuring that purging is not in effect.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  // Adding job_id to to all of the queues and saving the iterators.
  std::list<std::pair<JobId, std::list<JobId>::iterator>> iterators;
  for (auto const& prio : prio_list) {
    assert(prio <= _max_prio);

    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting job_id at prio level priority queue and saving iterator
    const auto iter =
        _priority_qs[prio].insert(_priority_qs[prio].end(), job_id);
    iterators.push_back({job_id, iter});
  }

  {
    // Locking this job map as insert can cause iterator invalidation.
    std::lock_guard<std::mutex> lock_pq(_job_map_mutex);
    auto map_result = _job_mapper.insert({job_id, iterators});
    // Checking if this job was already in the map.
    if (map_result.second == false) {
      // Appending "iterators" to the end of the the maps iterator vector.
      map_result.first->second.insert(map_result.first->second.end(),
                                      iterators.begin(), iterators.end());
    }
  }

  // If one of the queues was empty but is not now, unlock the state mutex.
  for (auto const& prio : prio_list) {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
    if (_priority_qs[prio].size() == 1) _not_empty_mutexes[prio].unlock();
  }
}

void MultiQueue::Dequeue(Priority prio) {
  assert(prio <= _max_prio);

  // This lock is not being acquired right now as we do not want to block
  // purging for no reason.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex,
                                                       std::defer_lock);

  // It should impossible to leave this loop without having acquired the
  // no_purging lock and having ensured that there is at least a single item in
  // the queue.
  while (true) {
    // Ensuring that there is at least a single item in this queue.
    _not_empty_mutexes[prio].lock();

    // Ensuring that purging is not in effect.
    no_pruging.lock();

    // Protecting the race condition where there was an item in the queue when
    // we flipped the _not_empty_mutex but a purge call happened right before
    // we acquired the no_purging lock.
    if (_priority_qs[prio].size() == kMinimumQueueSize)
      no_pruging.unlock();
    else
      break;
  }

  // Locking this priority queue
  std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
}

// template <typename T>
// struct JobMap<T>* MultiQueue<T>::GetJobMap(JobId job_id) {
//   auto search = _job_mapper.find(job_id);
//   if (search == _job_mapper.end())
//     return nullptr;
//   else
//     return &search->second;
// }

// template <typename T>
// bool MultiQueue<T>::Purge(JobId job_id) {
//   JobMap<T>* job_map = GetJobMap(job_id);

//   if (job_map == nullptr)
//     return false;

//   // for (auto const& instance: job_map.instances) {

//   // }

//   return true;
// }

}  // namespace duplicate_aware_scheduling
