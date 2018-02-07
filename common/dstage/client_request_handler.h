#ifndef DANS02_CLIENT_REQUEST_HANDLER_H
#define DANS02_CLIENT_REQUEST_HANDLER_H

#include <functional>
#include <memory>
#include <string>

#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

struct ReqData {
  std::vector<std::string> ip_addresses;
  std::vector<std::string> ports;
  std::function<void(int)> done;
};

struct ReqDataInternal {
  int soc;
  std::function<void(int)> done;
};

// class RequestDispatcher : public Dispatcher<ReqData, ReqDataInternal> {
//  public:
//   RequestDispatcher(Priority max_priority);

//  protected:
//   UniqConstJobPtr<ReqDataInternal> DuplicateAndConvert(
//       const Job<ReqData>* job_in, Priority prio, unsigned duplication)
//       override;

//   void SendToMultiQueue(
//       UniqConstJobPtr<ReqDataInternal> duplicate_job) override;
// };

}  // namespace dans

#endif  // DANS02_CLIENT_REQUEST_HANDLER_H