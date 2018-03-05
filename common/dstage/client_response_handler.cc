#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>

#include "common/dstage/client_response_handler.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
}  // namespace

namespace dans {

ResponseDispatcher::ResponseDispatcher(Priority max_priority)
    : Dispatcher<ResponseData, ResponseData>(max_priority) {}

void ResponseDispatcher::DuplicateAndEnqueue(
    UniqConstJobPtr<ResponseData> job_in, Priority max_prio,
    unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << job_in->priority;
  _multi_q_p->Enqueue(std::move(job_in));
}

ResponseScheduler::ResponseScheduler(std::vector<unsigned> threads_per_prio,
                                     bool set_thread_priority)
    // CommunicationHandlerInterface* comm_interface,
    // BaseDStage<ResponseData>* response_dstage)
    : Scheduler<ResponseData>(threads_per_prio, set_thread_priority),
      // _comm_interface(comm_interface),
      // _response_dstage(response_dstage),
      _destructing(false),
      _origin_dstage(nullptr) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

ResponseScheduler::~ResponseScheduler() {
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

void ResponseScheduler::RegisterOriginDStage(
    BaseDStage<ConnectData>* origin_dstage) {
  _origin_dstage = origin_dstage;
}

void ResponseScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  Protocol response;

  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedConstJobPtr<ResponseData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Response Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.soc;

    // Check if job has been purged
    if (job->job_data.purge_state->IsPurged()) {
      VLOG(2) << "Purged job_id=" << job->job_id
              << ", Priority=" << job->priority;
      close(job->job_data.soc);
      continue;
    }

    if (job->job_data.soc < 0) {
      errno = -job->job_data.soc;
      PLOG(WARNING) << "Dropped socket for job_id=" << job->job_id;
      continue;
    }

    CHECK_EQ(read(job->job_data.soc, &response, sizeof(Protocol)),
             sizeof(Protocol));
    if (job->job_data.purge_state->SetPurged()) {
      VLOG(2) << "Completed job_id=" << job->job_id
              << ", priority=" << job->priority;
      (*job->job_data.done)(job->priority, &response, sizeof(Protocol));
      close(job->job_data.soc);
      CHECK_NOTNULL(_origin_dstage);
      _origin_dstage->Purge(job->job_id);
    }

    // CallBack2 response(std::bind(&dans::ResponseScheduler::ResponseCallback,
    // this,
    //                              job, std::placeholders::_1,
    //                              std::placeholders::_2));
  }
}

void ResponseScheduler::ResponseCallback(
    SharedConstJobPtr<ResponseData> old_job, int soc, ReadyFor ready_for) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << old_job->job_data.soc;
  CHECK(ready_for.in) << "Failed to receive response for socket=" << soc;

  // Pass on job if it is not complete.
  if (!old_job->job_data.purge_state->IsPurged()) {
    ResponseData response_data = {soc,
                                  old_job->job_data.object_id,
                                  old_job->job_data.index,
                                  std::move(old_job->job_data.object),
                                  old_job->job_data.done,
                                  old_job->job_data.purge_state};

    auto response_job = std::make_unique<ConstJob<ResponseData>>(
        std::move(old_job->job_data),
        old_job->job_id, old_job->priority, old_job->duplication);
    // _response_dstage->Dispatch(std::move(response_job),
    //                           /*requested_dulpication=*/0);
  } else {
    VLOG(2) << "Purged job_id=" << old_job->job_id
            << ", Priority=" << old_job->priority;
  }
}

}  // namespace dans