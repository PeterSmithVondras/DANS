#include "common/dstage/multiqueue.h"

#include <cassert>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace duplicate_aware_scheduling {

template <typename Key, typename Value>
MultiQueue<Key, Value>::MultiQueue(unsigned max_priority)
    : _max_prio(max_priority),
      _not_empty_mutexes(_max_prio + 1),
      _pq_mutexes(_max_prio + 1),
      _priority_qs(_max_prio + 1) {
  for (unsigned i = 0; i < _max_prio; i++) {
    // Queues start empty and therefore "not empty" is locked.
    _not_empty_mutexes[i].lock();
  }
}

template <typename Key, typename Value>
void MultiQueue<Key, Value>::Enqueue(std::pair<Key, Value> key_value,
                                     Priority prio) {
  // Ensuring that purging is not in effect.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  // Adding value to to all of the queues and saving the iterators.
  std::list<
      std::pair<Priority, typename std::list<std::pair<Key, Value>>::iterator>>
      duplicate_list;

  {
    assert(prio <= _max_prio);

    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting value at prio level priority queue and saving iterator
    const auto iter =
        _priority_qs[prio].insert(_priority_qs[prio].end(), key_value);
    duplicate_list.push_back({prio, iter});
  }

  {
    // Locking this value map as insert can cause iterator invalidation.
    std::lock_guard<std::mutex> lock_pq(_value_map_mutex);
    auto map_result = _value_mapper.insert({key_value.first, duplicate_list});
    // Checking if this value was already in the map.
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

template <typename Key, typename Value>
Value MultiQueue<Key, Value>::Dequeue(Priority prio) {
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

  // Getting the value from the queue.
  std::pair<Key, Value> key_value;

  {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
    key_value = _priority_qs[prio].front();
    _priority_qs[prio].pop_front();

    // If the Q is not empty we know that it is safe to unblock at least one
    // more dequeue.
    if (!_priority_qs[prio].empty()) _not_empty_mutexes[prio].unlock();
  }

  // Locking value map as our iterator could be invalidated if there is an
  // insert.
  std::lock_guard<std::mutex> lock_pq(_value_map_mutex);
  auto value_map_itr_at_target = _value_mapper.find(key_value.first);
  assert(value_map_itr_at_target != _value_mapper.end());
  auto duplicate_list_iter = value_map_itr_at_target->second.begin();

  bool found = false;
  while (duplicate_list_iter != value_map_itr_at_target->second.end()) {
    if (duplicate_list_iter->first == prio) {
      value_map_itr_at_target->second.erase(duplicate_list_iter);
      found = true;
      break;
    }

    duplicate_list_iter++;
  }
  // We should always find what we removed from the queue.
  assert(found);

  // Removing entry from _value_mapper if it is now empty.
  if (value_map_itr_at_target->second.empty())
    _value_mapper.erase(value_map_itr_at_target);

  return key_value.second;
}

template <typename Key, typename Value>
std::list<Priority> MultiQueue<Key, Value>::Purge(Key key) {
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  std::list<Priority> purged;
  auto search = _value_mapper.find(key);

  // If value is already purged we can just return an empty list.
  if (search == _value_mapper.end()) return purged;

  for (std::pair<Priority,
                 typename std::list<std::pair<Key, Value>>::iterator> const&
           pair : search->second) {
    Priority prio = pair.first;
    _priority_qs[prio].erase(pair.second);
    purged.push_back(prio);
  }

  _value_mapper.erase(search);
  return purged;
}

template <typename Key, typename Value>
unsigned MultiQueue<Key, Value>::Size(Priority prio) {
  assert(prio <= _max_prio);
  return _priority_qs[prio].size();
}

template <typename Key, typename Value>
bool MultiQueue<Key, Value>::Empty(Priority prio) {
  assert(prio <= _max_prio);
  return _priority_qs[prio].empty();
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class MultiQueue<JobId, std::shared_ptr<const Job<JData>>>;

}  // namespace duplicate_aware_scheduling
