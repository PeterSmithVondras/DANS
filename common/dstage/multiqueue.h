#ifndef DANS02_DSTAGE_MULTIQUEUE_H
#define DANS02_DSTAGE_MULTIQUEUE_H

#include <list>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {

class MultiQueue {
 public:
  MultiQueue(unsigned number_of_qs);
  ~MultiQueue() {}

  void Enqueue(JobId job_id, std::vector<Priority> prio_list);

  // bool Purge(JobId job_id);

 protected:
  std::shared_timed_mutex _purge_shared_mutex;

  std::vector<std::mutex> _not_empty_mutexes;
  std::vector<std::mutex> _pq_mutexes;
  std::vector<std::list<JobId>> _priority_qs;

  std::mutex _job_map_mutex;
  std::unordered_map<JobId, std::vector<std::list<JobId>::iterator>>
      _job_mapper;
};

}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_MULTIQUEUE_H