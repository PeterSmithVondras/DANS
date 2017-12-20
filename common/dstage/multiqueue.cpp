#include "common/dstage/multiqueue.h"

#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace duplicate_aware_scheduling {

MultiQueue::MultiQueue(unsigned number_of_qs)
    : _not_empty_mutexes(number_of_qs), _pq_mutexes(number_of_qs) {
  (void)number_of_qs;
  std::list<JobId> q;
  for (unsigned i = 0; i < number_of_qs; i++) {
    // Setting up priority Queue.
    _priority_qs.push_back(q);

    // Queues start empty and therefore "not empty" is locked.
    _not_empty_mutexes[i].lock();
  }
}

void MultiQueue::Enqueue(JobId job_id, std::vector<Priority> prio_list) {
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);
  // std::lock_guard<std::mutex> lock

  std::list<std::list<JobId>::iterator> iterators;
  for (auto const& prio : prio_list) {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting job_id at prio level priority queue and saving iterator
    const auto inserted_item =
        _priority_qs[prio].insert(_priority_qs[prio].end(), job_id);
    iterators.push_back(inserted_item);
  }

  {
    // Locking this job map as insert can cause iterator invalidation.
    std::lock_guard<std::mutex> lock_pq(_job_map_mutex);
    auto map_result =
        _job_mapper.insert({job_id, iterators});
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
    if (_priority_qs[prio].size() == 1)
      _not_empty_mutexes[prio].unlock();
  }
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
