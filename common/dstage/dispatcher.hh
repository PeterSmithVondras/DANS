// This is an implementation file which is being included as a header so that
// we can have dynamic template specialization at compile time.
#ifndef DANS02_DISPATCHER_CC_IMPL__
#define DANS02_DISPATCHER_CC_IMPL__

#include "common/dstage/dispatcher.h"

#include <memory>

#include "glog/logging.h"

namespace {}  // namespace

namespace dans {

template <typename T_INPUT, typename T_INTERNAL>
Dispatcher<T_INPUT, T_INTERNAL>::Dispatcher(unsigned max_priority)
    : _running(false), _max_priority(max_priority), _multi_q_p(nullptr) {
  VLOG(4) << __PRETTY_FUNCTION__ << " max_priority=" << max_priority;
}

// Duplicates job and inserts into MultiQueue
template <typename T_INPUT, typename T_INTERNAL>
void Dispatcher<T_INPUT, T_INTERNAL>::Dispatch(UniqConstJobPtr<T_INPUT> job_p,
                                               unsigned requested_duplication) {
  VLOG(4) << __PRETTY_FUNCTION__
          << ((job_p == nullptr) ? " job_p=nullptr," : " job_id=")
          << ((job_p == nullptr) ? ' ' : job_p->job_id)
          << ((job_p == nullptr) ? ' ' : ',')
          << " requested_duplication=" << requested_duplication;
  CHECK(_running);
  if (job_p == nullptr) {
    LOG(WARNING) << "Dispatcher sent nullptr instead of a job with "
                 << "requested_duplication=" << requested_duplication;
    return;
  }
  CHECK_LE(job_p->priority, _max_priority);

  // Get lowest priority (highest value) appropriate.
  Priority max_prio = job_p->priority + requested_duplication > _max_priority
                          ? _max_priority
                          : job_p->priority + requested_duplication;

  DuplicateAndEnqueue(std::move(job_p), max_prio, max_prio - job_p->priority);
}

template <typename T_INPUT, typename T_INTERNAL>
void Dispatcher<T_INPUT, T_INTERNAL>::LinkMultiQ(
    BaseMultiQueue<T_INTERNAL>* multi_q_p) {
  VLOG(4) << __PRETTY_FUNCTION__;
  CHECK_NOTNULL(multi_q_p);
  _multi_q_p = multi_q_p;
  _running = true;
}

}  // namespace dans

#endif  // DANS02_DISPATCHER_CC_IMPL__