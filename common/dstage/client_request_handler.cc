#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "glog/logging.h"

namespace {
// ReqData req_data = {{"172.217.10.36"}, {"80"}, [](int foo) {}};
}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<ReqData, ReqDataInternal>(max_priority) {}

void RequestDispatcher::DuplicateAndEnqueue(UniqConstJobPtr<ReqData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " max_prio=" << max_prio
          << ", duplication= " << duplication;

  for (Priority prio = job_in->priority; prio <= max_prio; prio++) {
    ReqDataInternal req_data_internal = {/*soc=*/-1, job_in->job_data.done};
    auto duplicate_job_p = std::make_unique<const Job<ReqDataInternal>>(
        req_data_internal, job_in->job_id, prio, duplication);
    _multi_q_p->Enqueue(std::move(duplicate_job_p));
  }
}

}  // namespace dans