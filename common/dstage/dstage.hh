// This is an implementation file which is being included as a header so that
// we can have dynamic template specialization at compile time.
#ifndef DANS02_DSTAGE_CC_IMPL__
#define DANS02_DSTAGE_CC_IMPL__

#include "common/dstage/dstage.h"

#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "glog/logging.h"

namespace dans {

template <typename T_INPUT, typename T_INTERNAL>
DStage<T_INPUT, T_INTERNAL>::DStage(
    Priority max_priority, std::unique_ptr<BaseMultiQueue<T_INTERNAL>> multi_q,
    std::unique_ptr<BaseDispatcher<T_INPUT, T_INTERNAL>> dispatcher,
    std::unique_ptr<BaseScheduler<T_INTERNAL>> scheduler)
    : _max_priority(max_priority),
      _multi_q(std::move(multi_q)),
      _dispatcher(std::move(dispatcher)),
      _scheduler(std::move(scheduler)) {
  VLOG(4) << __PRETTY_FUNCTION__ << " max_priority=" << max_priority;
  // Linking scheduler first so that multiqueue has an outlet before a source.
  _scheduler->LinkMultiQ(_multi_q.get());
  _scheduler->Run();
  _dispatcher->LinkMultiQ(_multi_q.get());
}

template <typename T_INPUT, typename T_INTERNAL>
void DStage<T_INPUT, T_INTERNAL>::Dispatch(UniqJobPtr<T_INPUT> job_p,
                                           unsigned requested_duplication) {
  VLOG(4) << __PRETTY_FUNCTION__
          << ((job_p == nullptr) ? " job_p=nullptr," : " job_id=")
          << ((job_p == nullptr) ? ' ' : job_p->job_id)
          << ((job_p == nullptr) ? ' ' : ',')
          << " requested_duplication=" << requested_duplication;
  _dispatcher->Dispatch(std::move(job_p), requested_duplication);
}

template <typename T_INPUT, typename T_INTERNAL>
unsigned DStage<T_INPUT, T_INTERNAL>::Purge(JobId job_id) {
  VLOG(4) << __PRETTY_FUNCTION__ << " job_id=" << job_id;
  unsigned purged = _multi_q->Purge(job_id);
  purged += _scheduler->Purge(job_id);
  return purged;
}

}  // namespace dans

#endif  // DANS02_DSTAGE_CC_IMPL__