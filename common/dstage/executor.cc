#include "common/dstage/executor.h"
#include "glog/logging.h"

namespace dans {

Executor::ExecutorScheduler::ExecutorScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority)
    : Scheduler<ExecutorCallback>(threads_per_prio, set_thread_priority),
      _destructing(false) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

Executor::ExecutorScheduler::~ExecutorScheduler() {
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

void Executor::ExecutorScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedJobPtr<ExecutorCallback> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Executor running closure for job_id=" << job->job_data.id;
    job->job_data.closure();
  }
}

void Executor::Submit(ExecutorCallback task) {
  VLOG(4) << __PRETTY_FUNCTION__;
  auto job = std::make_unique<Job<ExecutorCallback>>(
      std::move(task), _jid_factory.CreateJobId(), /*priority=*/0,
      /*duplication*/ 0);
  Dispatch(std::move(job), /*requested_duplication=*/0);
}

}  // namespace dans