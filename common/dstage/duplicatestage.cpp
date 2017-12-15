#include "common/dstage/duplicatestage.h"

#include <vector>

#include "common/dstage/dstage.h"
#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {

template <typename T>
DuplicateStage<T>::DuplicateStage(unsigned max_duplication_level,
                                  std::unique_ptr<Dispatcher<T>> dispatcher)
  : _max_duplication_level(max_duplication_level),
    _dispatcher(dispatcher) {
  std::list<JobMap<T>*> q;
  for (int i = 0; i < _max_duplication_level; i++) {
    _priority_qs.push_back(q);
  }
}

} // namespace duplicate_aware_scheduling