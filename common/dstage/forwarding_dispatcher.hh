#ifndef DANS02_FORWARDING_DISPATCHER_CC_IMPL__
#define DANS02_FORWARDING_DISPATCHER_CC_IMPL__

#include "common/dstage/forwarding_dispatcher.h"
#include "glog/logging.h"

namespace {}  // namespace

namespace dans {

template <typename JOB_TYPE>
ForwardingDispatcher<JOB_TYPE>::ForwardingDispatcher(Priority max_priority)
    : Dispatcher<JOB_TYPE, JOB_TYPE>(max_priority) {}

template <typename JOB_TYPE>
void ForwardingDispatcher<JOB_TYPE>::DuplicateAndEnqueue(
    UniqJobPtr<JOB_TYPE> job_in, Priority max_prio, unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << job_in->priority;
  // The "this" pointer is needed as the function DuplicateAndeEnqueue is a
  // non-dependent name and therefore does not implicitly know to check the base
  // class. Another option is to append Dispatcher<TYPE>:: to the protected base
  // member.
  this->_multi_q_p->Enqueue(std::move(job_in));
}

// template class ForwardingDispatcher<int>;

}  // namespace dans

#endif  // DANS02_FORWARDING_DISPATCHER_CC_IMPL__