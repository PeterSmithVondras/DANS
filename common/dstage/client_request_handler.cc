#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "glog/logging.h"

namespace {
std::string buf = "GET / HTTP/1.1\n\n";
const std::string request = "GET / HTTP/1.1\n\n";
const int kIndex = 0;
const int kSizeMB = 0;

using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<RequestData, RequestData>(max_priority) {}

void RequestDispatcher::DuplicateAndEnqueue(UniqJobPtr<RequestData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << job_in->priority;
  _multi_q_p->Enqueue(std::move(job_in));
}

RequestScheduler::RequestScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority,
    CommunicationHandlerInterface* comm_interface,
    BaseDStage<ResponseData>* response_dstage)
    : Scheduler<RequestData>(threads_per_prio, set_thread_priority),
      _comm_interface(comm_interface),
      _response_dstage(response_dstage),
      _destructing(false) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

RequestScheduler::~RequestScheduler() {
  VLOG(4) << __PRETTY_FUNCTION__;
  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }

  // Releasing threads from blocking MultiQueue call.
  if (_running) {
    _multi_q_p->ReleaseQueues();
  }
}

unsigned RequestScheduler::Purge(JobId job_id) {
  VLOG(4) << __PRETTY_FUNCTION__ << " job_id=" << job_id;
  return _response_dstage->Purge(job_id);
}

void RequestScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;

  Protocol request;

  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedJobPtr<RequestData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Request Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.connection->Socket();

    // Check if job has been purged
    if (job->job_data.purge_state->IsPurged()) {
      VLOG(2) << "Purged job_id=" << job->job_id
              << ", Priority=" << job->priority;
      continue;
    }

    request = {REQUEST_GETFILE, static_cast<int>(job->priority),
               job->job_data.object_id, kIndex, kSizeMB};
    int ret = send(job->job_data.connection->Socket(), &request,
                   sizeof(Protocol), /*flags=*/0);
    CHECK_EQ(ret, static_cast<int>(sizeof(Protocol)))
        << "Failed to send: socket=" << job->job_data.connection->Socket();

    CallBack2 response(std::bind(&dans::RequestScheduler::RequestCallback, this,
                                 job, std::placeholders::_1,
                                 std::placeholders::_2));
    _comm_interface->Monitor(job->job_data.connection->Socket(),
                             ReadyFor{/*in=*/true, /*out=*/false}, response);
  }
}

void RequestScheduler::RequestCallback(SharedJobPtr<RequestData> old_job,
                                       int soc, ReadyFor ready_for) {
  VLOG(4) << __PRETTY_FUNCTION__
          << " soc=" << old_job->job_data.connection->Socket();
  CHECK(ready_for.in) << "Failed to receive response for socket="
                      << old_job->job_data.connection->Socket();

  // Pass on job if it is not complete.
  if (!old_job->job_data.purge_state->IsPurged() && soc >= 0) {
    ResponseData response_data = {std::move(old_job->job_data.connection),
                                  old_job->job_data.object_id,
                                  /*index=*/0,
                                  /*object=*/nullptr,
                                  old_job->job_data.done,
                                  old_job->job_data.purge_state};

    auto response_job = std::make_unique<Job<ResponseData>>(
        std::move(response_data), old_job->job_id, old_job->priority,
        old_job->duplication);
    _response_dstage->Dispatch(std::move(response_job),
                               /*requested_dulpication=*/0);
  } else if(soc < 0) {
    errno = -soc;
    PLOG(WARNING) << "Dropped socket for job_id=" << old_job->job_id;
  } else {
    VLOG(2) << "Purged job_id=" << old_job->job_id
            << ", Priority=" << old_job->priority;
  }
}

}  // namespace dans