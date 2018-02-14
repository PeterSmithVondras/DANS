#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>

#include "common/dstage/client_response_handler.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
int kReadSize = 15;
}  // namespace

namespace dans {

ResponseScheduler::ResponseScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority)
    // CommunicationHandlerInterface* comm_interface,
    // BaseDStage<RequestData>* response_dstage)
    : Scheduler<RequestData>(threads_per_prio, set_thread_priority),
      // _comm_interface(comm_interface),
      // _response_dstage(response_dstage),
      _destructing(false) {
  VLOG(3) << __PRETTY_FUNCTION__;
}

ResponseScheduler::~ResponseScheduler() {
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

void ResponseScheduler::StartScheduling(Priority prio) {
  VLOG(3) << __PRETTY_FUNCTION__ << " prio=" << prio;
  char buf[15];
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedConstJobPtr<RequestData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Response Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.soc;

    if (job->job_data.soc < 0) {
      errno = -job->job_data.soc;
      PLOG(WARNING) << "Dropped socket for job_id=" << job->job_id;
      continue;
    }

    CHECK_EQ(read(job->job_data.soc, buf, kReadSize), kReadSize);
    VLOG(2) << "Read from server: " << buf;

    // CallBack2 response(std::bind(&dans::ResponseScheduler::ResponseCallback, this,
    //                              job, std::placeholders::_1,
    //                              std::placeholders::_2));
  }
}

void ResponseScheduler::ResponseCallback(SharedConstJobPtr<RequestData> old_job,
                                       int soc, ReadyFor ready_for) {
  VLOG(3) << __PRETTY_FUNCTION__ << " soc=" << old_job->job_data.soc;
  CHECK(ready_for.in) << "Failed to receive response for socket=" << soc;
  auto response_job = std::make_unique<ConstJob<RequestData>>(
      RequestData{old_job->job_data.done, soc}, old_job->job_id,
      old_job->priority, old_job->duplication);
  // _response_dstage->Dispatch(std::move(response_job),
  //                           /*requested_dulpication=*/0);
}

}  // namespace dans