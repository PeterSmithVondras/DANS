#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <list>
#include <unordered_map>
#include <memory>
#include <vector>

#include "common/dstage/dstage.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {
template <typename T>
class DuplicateStage : public DStage<T> {
public:
  DuplicateStage(unsigned max_duplication_level,
                 std::unique_ptr<Dispatcher<T>> dispatcher);
  ~DuplicateStage() {}

protected:
  std::vector<std::list<JobMap<T>*>> _priority_qs;
  std::unordered_map<JobId, struct JobMap<T>> _job_mapper;
  const unsigned _max_duplication_level;

  std::unique_ptr<Dispatcher<T>> _dispatcher;
  // std::unique_ptr<scheduler<T>> _scheduler;
};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DUPLICATESTAGE_H