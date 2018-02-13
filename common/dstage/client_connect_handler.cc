#include <memory>
#include <string>

#include "common/dstage/client_connection_handler.h"
#include "glog/logging.h"

namespace {}  // namespace

namespace dans {

ConnectDispatcher::ConnectDispatcher(Priority max_priority)
    : Dispatcher<ConnectData, ConnectDataInternal>(max_priority) {}

void ConnectDispatcher::DuplicateAndEnqueue(UniqConstJobPtr<ConnectData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(4) << __PRETTY_FUNCTION__ << " max_prio=" << max_prio
          << ", duplication= " << duplication;
  CHECK(job_in->job_data.ip_addresses.size() > duplication)
      << "There are " << job_in->job_data.ip_addresses.size()
      << " ip_addresses, but we need at least " << duplication + 1;
  CHECK(job_in->job_data.ports.size() > duplication)
      << "There are " << job_in->job_data.ip_addresses.size()
      << " ports, but we need at least " << duplication + 1;

  Priority prio;
  int i;
  VLOG(1) << "ip_addresses size=" << job_in->job_data.ip_addresses.size();
  for (i = 0, prio = job_in->priority; prio <= max_prio; i++, prio++) {
    VLOG(1) << "i=" << i << ", prio=" << prio;
    ConnectDataInternal req_data_internal = {job_in->job_data.ip_addresses[i],
                                             job_in->job_data.ports[i],
                                             job_in->job_data.done};
    auto duplicate_job_p = std::make_unique<const Job<ConnectDataInternal>>(
        req_data_internal, job_in->job_id, prio, duplication);
    _multi_q_p->Enqueue(std::move(duplicate_job_p));
  }
}

ConnectScheduler::ConnectScheduler(std::vector<unsigned> threads_per_prio,
                                   bool set_thread_priority)
    : Scheduler<ConnectDataInternal>(threads_per_prio, set_thread_priority),
      _destructing(false) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

ConnectScheduler::~ConnectScheduler() {
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

void ConnectScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    UniqConstJobPtr<ConnectDataInternal> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Connect Handler Scheduler got job_id=" << job->job_id
            << ", ip=" << job->job_data.ip << ", port=" << job->job_data.port;
  }
}

}  // namespace dans