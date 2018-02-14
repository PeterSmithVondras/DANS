#include <memory>
#include <string>

#include "common/dstage/client_connect_handler.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
}  // namespace

namespace dans {

ConnectDispatcher::ConnectDispatcher(Priority max_priority)
    : Dispatcher<ConnectData, ConnectDataInternal>(max_priority) {}

void ConnectDispatcher::DuplicateAndEnqueue(UniqConstJobPtr<ConnectData> job_in,
                                            Priority max_prio,
                                            unsigned duplication) {
  VLOG(3) << __PRETTY_FUNCTION__ << " max_prio=" << max_prio
          << ", duplication= " << duplication;
  CHECK(job_in->job_data.ip_addresses.size() > duplication)
      << "There are " << job_in->job_data.ip_addresses.size()
      << " ip_addresses, but we need at least " << duplication + 1;
  CHECK(job_in->job_data.ports.size() > duplication)
      << "There are " << job_in->job_data.ip_addresses.size()
      << " ports, but we need at least " << duplication + 1;

  Priority prio;
  int i;
  VLOG(2) << "ip_addresses size=" << job_in->job_data.ip_addresses.size();
  for (i = 0, prio = job_in->priority; prio <= max_prio; i++, prio++) {
    VLOG(2) << "duplicate_job=" << i << ", prio=" << prio;
    ConnectDataInternal req_data_internal = {job_in->job_data.ip_addresses[i],
                                             job_in->job_data.ports[i],
                                             job_in->job_data.done};
    auto duplicate_job_p = std::make_unique<const Job<ConnectDataInternal>>(
        req_data_internal, job_in->job_id, prio, duplication);
    _multi_q_p->Enqueue(std::move(duplicate_job_p));
  }
}

ConnectScheduler::ConnectScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority,
    CommunicationHandlerInterface* comm_interface,
    BaseDStage<RequestData>* request_dstage)
    : Scheduler<ConnectDataInternal>(threads_per_prio, set_thread_priority),
      _comm_interface(comm_interface),
      _request_dstage(request_dstage),
      _destructing(false) {
  VLOG(3) << __PRETTY_FUNCTION__;
}

ConnectScheduler::~ConnectScheduler() {
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

void ConnectScheduler::StartScheduling(Priority prio) {
  VLOG(3) << __PRETTY_FUNCTION__ << " prio=" << prio;
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    // Convert the UniqConstJobPtr to SharedConstJobPtr to allow capture in
    // closure. Note that uniq_ptr's are are hard/impossible to capture using
    // std::bind as they do not have a copy constructor.
    SharedConstJobPtr<ConnectDataInternal> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Connect Handler Scheduler got job_id=" << job->job_id
            << ", ip=" << job->job_data.ip << ", port=" << job->job_data.port;

    CallBack2 connected(std::bind(&dans::ConnectScheduler::ConnectCallback,
                                  this, job, std::placeholders::_1,
                                  std::placeholders::_2));

    _comm_interface->Connect(job->job_data.ip, job->job_data.port, connected);
  }
}

void ConnectScheduler::ConnectCallback(
    SharedConstJobPtr<ConnectDataInternal> old_job, int soc,
    ReadyFor ready_for) {
  VLOG(3) << __PRETTY_FUNCTION__ << " soc=" << soc;
  CHECK(ready_for.out) << "Failed to create TCP connection for socket=" << soc;
  CHECK(!ready_for.in) << "Failed to create TCP connection for socket=" << soc;
  auto request_job = std::make_unique<ConstJob<RequestData>>(
      RequestData{old_job->job_data.done, soc}, old_job->job_id,
      old_job->priority, old_job->duplication);
  _request_dstage->Dispatch(std::move(request_job),
                            /*requested_dulpication=*/0);
}

}  // namespace dans