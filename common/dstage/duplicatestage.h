#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <memory>
#include <mutex>
#include <vector>

#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace duplicate_aware_scheduling {

template <typename T>
class DuplicateStage : public DStage<T> {
 public:
  DuplicateStage(unsigned max_duplication_level,
                 std::unique_ptr<Dispatcher<T>> dispatcher);
  ~DuplicateStage() {}

 protected:
  const unsigned _max_duplication_level;

  MultiQueue _multi_q;

  std::unordered_map<JobId, struct Job<T>> _active_jobs;

  std::unique_ptr<Dispatcher<T>> _dispatcher;
  // std::unique_ptr<scheduler<T>> _scheduler;
};
}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_DUPLICATESTAGE_H