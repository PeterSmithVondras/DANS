#ifndef DANS02_FORWARDING_DISPATCHER_H
#define DANS02_FORWARDING_DISPATCHER_H

#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"

namespace dans {

template <typename JOB_TYPE>
class ForwardingDispatcher : public Dispatcher<JOB_TYPE, JOB_TYPE> {
 public:
  ForwardingDispatcher(Priority max_priority);

 protected:
  void DuplicateAndEnqueue(UniqJobPtr<JOB_TYPE> job_in, Priority max_prio,
                           unsigned duplication) override;
};

}  // namespace dans

#include "common/dstage/forwarding_dispatcher.hh"

#endif  // DANS02_FORWARDING_DISPATCHER_H