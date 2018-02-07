#ifndef DANS02_CLIENT_REQUEST_HANDLER_H
#define DANS02_CLIENT_REQUEST_HANDLER_H

#include <memory>
#include <string>

#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

class RequestDispatcher : public Dispatcher<JData, JData> {
 public:
  RequestDispatcher(Priority max_priority);

 protected:
  UniqConstJobPtr<JData> DuplicateAndConvert(const Job<JData>* job_in,
                                             Priority prio,
                                             unsigned duplication) override;

  void SendToMultiQueue(UniqConstJobPtr<JData> duplicate_job) override;
};

}  // namespace dans

#endif  // DANS02_CLIENT_REQUEST_HANDLER_H