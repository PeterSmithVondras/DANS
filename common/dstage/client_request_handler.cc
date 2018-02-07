#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "glog/logging.h"

namespace {
using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
auto kGenericData = std::make_shared<JData>(5);
// const unsigned kGenericDuplication = 0;

  ReqData req_data = {{"172.217.10.36"}, {"80"}, [](int foo){}};
}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<ReqData, ReqDataInternal>(max_priority) {}

UniqConstJobPtr<ReqDataInternal> RequestDispatcher::DuplicateAndConvert(
    const Job<ReqData>* job_in, Priority prio, unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio
          << ", duplication= " << duplication;
  return std::make_unique<const Job<ReqDataInternal>>(
      job_in->job_data, job_in->job_id, prio, duplication);
}

// void RequestDispatcher::SendToMultiQueue(
//     UniqConstJobPtr<ReqDataInternal> duplicate_job) {
//   _multi_q_p->Enqueue(std::move(duplicate_job));
// }

}  // namespace dans