#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"

namespace {
using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
auto kGenericData = std::make_shared<JData>(5);
// const unsigned kGenericDuplication = 0;

}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<JData, JData>(max_priority) {}

UniqConstJobPtr<JData> RequestDispatcher::DuplicateAndConvert(
    const Job<JData>* job_in, Priority prio, unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio
          << ", duplication= " << duplication;
  return std::make_unique<const Job<JData>>(job_in->job_data, job_in->job_id,
                                            prio, duplication);
}
void RequestDispatcher::SendToMultiQueue(UniqConstJobPtr<JData> duplicate_job) {
  _multi_q_p->Enqueue(std::move(duplicate_job));
}

}  // namespace dans