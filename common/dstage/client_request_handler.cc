#include <memory>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "glog/logging.h"

namespace {}  // namespace

namespace dans {

RequestDispatcher::RequestDispatcher(Priority max_priority)
    : Dispatcher<RequestData, RequestData>(max_priority) {}

void RequestDispatcher::DuplicateAndEnqueue(UniqConstJobPtr<RequestData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(3) << __PRETTY_FUNCTION__ << " max_prio=" << max_prio
          << ", duplication= " << duplication;
  _multi_q_p->Enqueue(std::move(job_in));
}

RequestScheduler::RequestScheduler(std::vector<unsigned> threads_per_prio,
                                   bool set_thread_priority)
    : Scheduler<RequestData>(threads_per_prio, set_thread_priority),
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

    UniqConstJobPtr<RequestData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Request Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.soc;
  }
}

}  // namespace dans