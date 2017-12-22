#include "common/dstage/multiqueue.h"

#include <cassert>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace duplicate_aware_scheduling {

template <typename T>
MultiQueue<T>::MultiQueue(unsigned max_priority)
    : _max_prio(max_priority),
      _not_empty_mutexes(_max_prio + 1),
      _pq_mutexes(_max_prio + 1),
      _priority_qs(_max_prio + 1) {
  for (unsigned i = 0; i < _max_prio; i++) {
    // Queues start empty and therefore "not empty" is locked.
    _not_empty_mutexes[i].lock();
  }
}

template <typename T>
void MultiQueue<T>::Enqueue(std::shared_ptr<const Job<T>> job, Priority prio) {
  // Ensuring that purging is not in effect.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  // Adding job to to all of the queues and saving the iterators.
  std::list<std::pair<
      Priority, typename std::list<std::shared_ptr<const Job<T>>>::iterator>>
      duplicate_list;

  {
    assert(prio <= _max_prio);

    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting job at prio level priority queue and saving iterator
    const auto iter = _priority_qs[prio].insert(_priority_qs[prio].end(), job);
    duplicate_list.push_back({prio, iter});
  }

  {
    // Locking this job map as insert can cause iterator invalidation.
    std::lock_guard<std::mutex> lock_pq(_job_map_mutex);
    auto map_result = _job_mapper.insert({job->job_id, duplicate_list});
    // Checking if this job was already in the map.
    if (map_result.second == false) {
      // Appending "duplicate_list" to the end of the the maps iterator
      // vector.
      map_result.first->second.insert(map_result.first->second.end(),
                                      duplicate_list.begin(),
                                      duplicate_list.end());
    }
  }

  // If one of the queues was empty but is not now, unlock the state mutex.
  std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
  if (_priority_qs[prio].size() == 1) _not_empty_mutexes[prio].unlock();
}

template <typename T>
std::shared_ptr<const Job<T>> MultiQueue<T>::Dequeue(Priority prio) {
  assert(prio <= _max_prio);

  // This lock is not being acquired right now as we do not want to block
  // purging for no reason.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex,
                                                       std::defer_lock);

  // It should impossible to leave this loop without having acquired the
  // no_purging lock and having ensured that there is at least a single item

  // the queue.
  while (true) {
    // Ensuring that there is at least a single item in this queue.
    _not_empty_mutexes[prio].lock();

    // Ensuring that purging is not in effect.
    no_pruging.lock();

    // Protecting the race condition where there was an item in the queue when
    // we flipped the _not_empty_mutex but a purge call happened right before we
    // acquired the no_purging lock.
    if (_priority_qs[prio].empty())
      no_pruging.unlock();
    else
      break;
  }

  // Getting the job from the queue.
  std::shared_ptr<const Job<T>> job;

  {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
    job = _priority_qs[prio].front();
    _priority_qs[prio].pop_front();

    // If the Q is not empty we know that it is safe to unblock at least one
    // more dequeue.
    if (!_priority_qs[prio].empty()) _not_empty_mutexes[prio].unlock();
  }

  // Locking job map as our iterator could be invalidated if there is an insert.
  std::lock_guard<std::mutex> lock_pq(_job_map_mutex);
  auto job_map_itr_at_target = _job_mapper.find(job->job_id);
  assert(job_map_itr_at_target != _job_mapper.end());
  auto duplicate_list_iter = job_map_itr_at_target->second.begin();

  bool found = false;
  while (duplicate_list_iter != job_map_itr_at_target->second.end()) {
    if (duplicate_list_iter->first == prio) {
      job_map_itr_at_target->second.erase(duplicate_list_iter);
      found = true;
      break;
    }

    duplicate_list_iter++;
  }
  // We should always find what we removed from the queue.
  assert(found);

  // Removing entry from _job_mapper if it is now empty.
  if (job_map_itr_at_target->second.empty())
    _job_mapper.erase(job_map_itr_at_target);

  return job;
}

template <typename T>
std::list<Priority> MultiQueue<T>::Purge(JobId job_id) {
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  std::list<Priority> purged;
  auto search = _job_mapper.find(job_id);

  // If job is already purged we can just return an empty list.
  if (search == _job_mapper.end()) return purged;

  for (std::pair<Priority, typename std::list<std::shared_ptr<const Job<T>>>::
                               iterator> const& pair : search->second) {
    Priority prio = pair.first;
    _priority_qs[prio].erase(pair.second);
    purged.push_back(prio);
  }

  _job_mapper.erase(search);
  return purged;
}

template <typename T>
unsigned MultiQueue<T>::Size(Priority prio) {
  assert(prio <= _max_prio);
  return _priority_qs[prio].size();
}

template <typename T>
bool MultiQueue<T>::Empty(Priority prio) {
  assert(prio <= _max_prio);
  return _priority_qs[prio].empty();
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class MultiQueue<JData>;

}  // namespace duplicate_aware_scheduling
