#include "common/dstage/dstage.h"

#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "glog/logging.h"

namespace dans {

template <typename T>
DStage<T>::DStage(Priority max_priority,
                  std::unique_ptr<BaseMultiQueue<T>> multi_q,
                  std::unique_ptr<BaseDispatcher<T>> dispatcher,
                  std::unique_ptr<BaseScheduler<T>> scheduler)
    : _max_priority(max_priority),
      _multi_q(std::move(multi_q)),
      _dispatcher(std::move(dispatcher)),
      _scheduler(std::move(scheduler)) {
  // Linking scheduler first so that multiqueue has an outlet before a source.
  _scheduler->LinkMultiQ(_multi_q.get());
  _scheduler->Run();
  _dispatcher->LinkMultiQ(_multi_q.get());
}

template <typename T>
void DStage<T>::Dispatch(UniqConstJobPtr<T> job,
                         unsigned requested_duplication) {
  _dispatcher->Dispatch(std::move(job), requested_duplication);
}

template <typename T>
std::list<UniqConstJobPtr<T>> DStage<T>::Purge(JobId job_id) {
  std::list<UniqConstJobPtr<T>> purged = _multi_q->Purge(job_id);
  purged.splice(purged.end(), _scheduler->Purge(job_id));
  return purged;
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class DStage<JData>;

}  // namespace dans