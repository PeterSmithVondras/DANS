#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "glog/logging.h"

namespace {
std::string buf = "GET / HTTP/1.1\n\n";

using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<RequestData, RequestData>(max_priority) {}

void RequestDispatcher::DuplicateAndEnqueue(UniqConstJobPtr<RequestData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(3) << __PRETTY_FUNCTION__ << " prio=" << job_in->priority;
  _multi_q_p->Enqueue(std::move(job_in));
}

RequestScheduler::RequestScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority,
    CommunicationHandlerInterface* comm_interface,
    BaseDStage<RequestData>* response_dstage)
    : Scheduler<RequestData>(threads_per_prio, set_thread_priority),
      _comm_interface(comm_interface),
      _response_dstage(response_dstage),
      _destructing(false) {
  VLOG(3) << __PRETTY_FUNCTION__;
}

RequestScheduler::~RequestScheduler() {
  VLOG(3) << __PRETTY_FUNCTION__;
  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }

  // Releasing threads from blocking MultiQueue call.
  if (_running) {
    _multi_q_p->ReleaseQueues();
  }
}

void RequestScheduler::StartScheduling(Priority prio) {
  VLOG(3) << __PRETTY_FUNCTION__ << " prio=" << prio;
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedConstJobPtr<RequestData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Request Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.soc;

    if (job->job_data.soc < 0) {
      errno = -job->job_data.soc;
      PLOG(WARNING) << "Dropped socket for job_id=" << job->job_id;
      continue;
    }

    int ret = send(job->job_data.soc, buf.c_str(), buf.length(), /*flags=*/0);
    CHECK_EQ(ret, static_cast<int>(buf.length()))
        << "Failed to send: socket=" << job->job_data.soc << "\n"
        << "Tried to send: " << buf;

    CallBack2 response(std::bind(&dans::RequestScheduler::RequestCallback, this,
                                 job, std::placeholders::_1,
                                 std::placeholders::_2));
    _comm_interface->Monitor(job->job_data.soc,
                             ReadyFor{/*in=*/true, /*out=*/false}, response);
  }
}

void RequestScheduler::RequestCallback(SharedConstJobPtr<RequestData> old_job,
                                       int soc, ReadyFor ready_for) {
  VLOG(3) << __PRETTY_FUNCTION__ << " soc=" << old_job->job_data.soc;
  CHECK(ready_for.in) << "Failed to receive response for socket=" << soc;
  auto response_job = std::make_unique<ConstJob<RequestData>>(
      RequestData{old_job->job_data.done, soc}, old_job->job_id,
      old_job->priority, old_job->duplication);
  _response_dstage->Dispatch(std::move(response_job),
                             /*requested_dulpication=*/0);
}

}  // namespace dans