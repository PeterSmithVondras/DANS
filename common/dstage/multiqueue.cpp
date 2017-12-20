#include "common/dstage/multiqueue.h"

#include <list>
#include <unordered_map>
#include <vector>

namespace duplicate_aware_scheduling {

MultiQueue::MultiQueue(unsigned number_of_qs)
    : _not_empty_mutexes(number_of_qs), _pq_mutexes(number_of_qs) {
  (void)number_of_qs;
  std::list<JobId> q;
  for (unsigned i = 0; i < number_of_qs; i++) {
    _priority_qs.push_back(q);
  }
}

void MultiQueue::Enqueue(JobId job_id, std::vector<Priority> prio_list) {
  auto map =
      _job_mapper.insert({job_id, std::vector<std::list<JobId>::iterator>()});

  for (auto const& prio : prio_list) {
    const auto inserted_item =
        _priority_qs[prio].insert(_priority_qs[prio].end(), job_id);
    map.first->second.push_back(inserted_item);
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
