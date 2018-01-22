#include "common/dstage/duplicatestage.h"

#include "common/dstage/dstage.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
DuplicateStage<T>::DuplicateStage(Priority max_priority,
                                  std::unique_ptr<Dispatcher<T>> dispatcher,
                                  std::unique_ptr<Scheduler<T>> scheduler)
    : _max_priority(max_priority),
      _multi_q(_max_priority),
      _dispatcher(std::move(dispatcher)),
      _scheduler(std::move(scheduler)) {
  // Linking scheduler first so that multiqueue has an outlet before a source.
  _scheduler->LinkMultiQ(&_multi_q);
  _scheduler->Run();
  _dispatcher->LinkMultiQ(&_multi_q);
}

template <typename T>
void DuplicateStage<T>::Dispatch(UniqConstJobPtr<T> job,
                                 unsigned requested_duplication) {
  _dispatcher->Dispatch(std::move(job), requested_duplication);
}

template <typename T>
std::list<UniqConstJobPtr<T>> DuplicateStage<T>::Purge(JobId job_id) {
  std::list<UniqConstJobPtr<T>> purged = _multi_q.Purge(job_id);
  purged.splice(purged.end(), _scheduler->Purge(job_id));
  return purged;
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class DuplicateStage<JData>;

}  // namespace dans