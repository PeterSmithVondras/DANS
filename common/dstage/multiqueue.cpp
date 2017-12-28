#include "common/dstage/multiqueue.h"

#include <cassert>
#include <list>
#include <memory>
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
void MultiQueue<T>::Enqueue(UniqJobPtr<T> job_p) {
  // Ensuring that purging is not in effect.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  Priority prio = job_p->priority;
  JobId job_id = job_p->job_id;
  // Adding value to the queue and saving the iterator.
  std::list<std::pair<UniqJobPtr<T>, typename std::list<JobId>::iterator>>
      duplicate_list;
  {
    assert(prio <= _max_prio);

    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting value at prio level priority queue and saving iterator
    const auto iter =
        _priority_qs[prio].insert(_priority_qs[prio].end(), job_id);
    duplicate_list.push_back({std::move(job_p), iter});
  }

  {
    // Locking this value map as insert can cause iterator invalidation.
    std::lock_guard<std::mutex> lock_pq(_value_map_mutex);
    auto search = _value_mapper.find(job_id);
    if (search == _value_mapper.end()) {
      _value_mapper.insert(std::make_pair(job_id, std::move(duplicate_list)));
    } else {
      // Appending "duplicate_list" to the end of the the maps iterator
      // vector.
      search->second.insert(search->second.end(),
                            std::make_move_iterator(duplicate_list.begin()),
                            std::make_move_iterator(duplicate_list.end()));
    }
  }

  // If one of the queues was empty but is not now, unlock the state mutex.
  std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
  if (_priority_qs[prio].size() == 1) _not_empty_mutexes[prio].unlock();
}

template <typename T>
UniqJobPtr<T> MultiQueue<T>::Dequeue(Priority prio) {
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
    // we flipped the _not_empty_mutex but a purge call happened right before we
    // acquired the no_purging lock.
    if (_priority_qs[prio].empty())
      no_pruging.unlock();
    else
      break;
  }

  // Getting the value from the queue.
  JobId job_id;

  {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
    job_id = *_priority_qs[prio].begin();
    _priority_qs[prio].pop_front();

    // If the Q is not empty we know that it is safe to unblock at least one
    // more dequeue.
    if (!_priority_qs[prio].empty()) _not_empty_mutexes[prio].unlock();
  }

  // Locking value map as our iterator could be invalidated if there is an
  // insert.
  std::lock_guard<std::mutex> lock_pq(_value_map_mutex);
  auto search = _value_mapper.find(job_id);
  assert(search != _value_mapper.end());
  auto duplicate_list_iter = search->second.begin();

  UniqJobPtr<T> job_p;
  bool found = false;
  while (duplicate_list_iter != search->second.end()) {
    if (duplicate_list_iter->first->priority == prio) {
      job_p = std::move(duplicate_list_iter->first);
      search->second.erase(duplicate_list_iter);
      found = true;
      break;
    }

    duplicate_list_iter++;
  }
  // We should always find what we removed from the queue.
  assert(found);

  // Removing entry from _value_mapper if it is now empty.
  if (search->second.empty()) _value_mapper.erase(search);

  return job_p;
}

template <typename T>
std::list<UniqJobPtr<T>> MultiQueue<T>::Purge(JobId job_id) {
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  std::list<UniqJobPtr<T>> purged;
  auto search = _value_mapper.find(job_id);

  // If value is already purged we can just return an empty list.
  if (search == _value_mapper.end()) return purged;

  auto duplicate_list_iter = search->second.begin();
  while (duplicate_list_iter != search->second.end()) {
    UniqJobPtr<T> job_p = std::move(duplicate_list_iter->first);
    _priority_qs[job_p->priority].erase(duplicate_list_iter->second);
    purged.push_back(std::move(job_p));

    duplicate_list_iter++;
  }

  _value_mapper.erase(search);
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
