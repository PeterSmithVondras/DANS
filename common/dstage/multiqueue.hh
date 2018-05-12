// This is an implementation file which is being included as a header so that
// we can have dynamic template specialization at compile time.
#ifndef DANS02_MULTIQUEUE_CC_IMPL__
#define DANS02_MULTIQUEUE_CC_IMPL__

#include "common/dstage/multiqueue.h"

#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "glog/logging.h"

namespace dans {

template <typename T>
MultiQueue<T>::MultiQueue(unsigned max_priority)
    : _max_prio(max_priority),
      _released(false),
      _not_empty_mutexes(_max_prio + 1),
      _pq_mutexes(_max_prio + 1),
      _priority_qs(_max_prio + 1),
      _release_one(_max_prio + 1, false) {
  VLOG(4) << __PRETTY_FUNCTION__ << " max_priority=" << max_priority;
  for (unsigned i = 0; i < _max_prio; i++) {
    // Queues start empty and therefore "not empty" is locked.
    _not_empty_mutexes[i].lock();
  }
}

template <typename T>
MultiQueue<T>::~MultiQueue() {
  VLOG(4) << __PRETTY_FUNCTION__;
}

template <typename T>
std::string MultiQueue<T>::DescribeQ(Priority prio) {
  std::ostringstream description;
  description << "prio=" << std::to_string(prio) << "(";
  for (auto const& id : _priority_qs[prio]) {
    description << "j" << std::to_string(id) << "<-";
  }
  description << ")";
  return description.str();
}

template <typename T>
std::string MultiQueue<T>::DescribeMapper() {
  std::ostringstream description;
  description << "mapper={";
  for (auto const& entry : _value_mapper) {
    description << "j" << std::to_string(entry.first) << "(";
    for (auto const& item : entry.second) {
      description << "p" << std::to_string(item.first->priority) << ",";
    }
    description << ") ";
  }
  description << "}";
  return description.str();
}

template <typename T>
std::string MultiQueue<T>::Describe() {
  std::ostringstream description;
  for (Priority i = 0; i < _priority_qs.size(); i++) {
    description << DescribeQ(i) << " ";
  }
  description << "while " << DescribeMapper();

  return description.str();
}

template <typename T>
void MultiQueue<T>::Enqueue(UniqJobPtr<T> job_p) {
  VLOG(4) << __PRETTY_FUNCTION__
          << ((job_p == nullptr) ? " job_p=nullptr," : " job_id=")
          << ((job_p == nullptr) ? ' ' : job_p->job_id)
          << ((job_p == nullptr) ? ' ' : ',');
  CHECK(job_p != nullptr) << Describe();

  // Ensuring that purging is not in effect.
  std::shared_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  Priority prio = job_p->priority;
  JobId job_id = job_p->job_id;
  // Adding value to the queue and saving the iterator.
  std::list<std::pair<UniqJobPtr<T>, typename std::list<JobId>::iterator>>
      duplicate_list;
  std::unique_lock<std::mutex> lock_vm(_value_map_mutex, std::defer_lock);
  {
    CHECK_LE(prio, _max_prio)
        << "prio=" << prio << "while max_prio=" << _max_prio;

    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);

    // Inserting value at prio level priority queue and saving iterator
    const auto iter =
        _priority_qs[prio].insert(_priority_qs[prio].end(), job_id);
    duplicate_list.push_back({std::move(job_p), iter});
    // If one of the queues was empty but is not now, unlock the state mutex.
    if (_priority_qs[prio].size() == 1) _not_empty_mutexes[prio].unlock();
    // Locking the value map as insert can cause iterator invalidation.
    lock_vm.lock();
  }

  {
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
}

template <typename T>
UniqJobPtr<T> MultiQueue<T>::Dequeue(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  CHECK_LE(prio, _max_prio)
      << "prio=" << prio << "while max_prio=" << _max_prio;

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
    if (_release_one[prio] == true) {
      _release_one[prio] = false;
      return nullptr;
    }

    // Ensuring that purging is not in effect.
    no_pruging.lock();

    // Protecting the race condition where there was an item in the queue when
    // we flipped the _not_empty_mutex but a purge call happened right before we
    // acquired the no_purging lock.
    if (_priority_qs[prio].empty() && _released) {
      _not_empty_mutexes[prio].unlock();
      return nullptr;
    } else if (_priority_qs[prio].empty())
      no_pruging.unlock();
    else
      break;
  }

  // Getting the value from the queue.
  JobId job_id;
  // Locking value map as our iterator could be invalidated if there is an
  // insert.
  std::unique_lock<std::mutex> lock_vm(_value_map_mutex, std::defer_lock);

  {
    // Locking this priority queue
    std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
    VLOG(4) << "DEQUEUE_prio=" << prio << ": " << DescribeQ(prio);
    job_id = *_priority_qs[prio].begin();
    _priority_qs[prio].pop_front();

    // If the Q is not empty we know that it is safe to unblock at least one
    // more dequeue.
    if (!_priority_qs[prio].empty()) _not_empty_mutexes[prio].unlock();
    lock_vm.lock();
  }

  VLOG(4) << "DEQUEUE_prio=" << prio << ": " << DescribeMapper();
  auto search = _value_mapper.find(job_id);
  CHECK(search != _value_mapper.end()) << DescribeMapper();
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
  CHECK(found);

  // Removing entry from _value_mapper if it is now empty.
  if (search->second.empty()) _value_mapper.erase(search);

  return job_p;
}

template <typename T>
unsigned MultiQueue<T>::Purge(JobId job_id) {
  VLOG(4) << __PRETTY_FUNCTION__ << " job_id=" << job_id;
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);

  unsigned purged = 0;
  auto search = _value_mapper.find(job_id);

  // If value is already purged we can just return an empty list.
  if (search == _value_mapper.end()) return purged;

  auto duplicate_list_iter = search->second.begin();
  while (duplicate_list_iter != search->second.end()) {
    UniqJobPtr<T> job_p = std::move(duplicate_list_iter->first);
    _priority_qs[job_p->priority].erase(duplicate_list_iter->second);
    purged++;

    duplicate_list_iter++;
  }

  _value_mapper.erase(search);
  VLOG(2) << "MultiQueue purged " << purged << " jobs.";
  return purged;
}

template <typename T>
unsigned MultiQueue<T>::Size(Priority prio) {
  CHECK_LE(prio, _max_prio)
      << "prio=" << prio << "while max_prio=" << _max_prio;
  return _priority_qs[prio].size();
}

template <typename T>
bool MultiQueue<T>::Empty(Priority prio) {
  CHECK_LE(prio, _max_prio);
  return _priority_qs[prio].empty();
}

template <typename T>
void MultiQueue<T>::ReleaseQueues() {
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);
  _released = true;

  for (Priority p = 0; p <= _max_prio; ++p) {
    if (_priority_qs[p].empty()) _not_empty_mutexes[p].unlock();
  }
}

template <typename T>
void MultiQueue<T>::ReleaseOne(Priority prio) {
  // Ensure complete control of the multiqueue
  std::unique_lock<std::shared_timed_mutex> no_pruging(_purge_shared_mutex);
  // Locking this priority queue
  std::lock_guard<std::mutex> lock_pq(_pq_mutexes[prio]);
  if (_priority_qs[prio].empty()) {
    _release_one[prio] = true;
    _not_empty_mutexes[prio].unlock();
  }
}

template <typename T>
unsigned MultiQueue<T>::MaxPriority() {
  return _max_prio;
}

}  // namespace dans

#endif  // DANS02_MULTIQUEUE_CC_IMPL__
