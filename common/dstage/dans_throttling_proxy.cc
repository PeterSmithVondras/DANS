#include <chrono>
#include <csignal>
#include <functional>

#include "common/dstage/dans_throttling_proxy.h"
#include "common/util/callback.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_uint64(primary_prio_port_in, 5010,
              "Port to listen for primary priority work.");
DEFINE_uint64(secondary_prio_port_in, 5011,
              "Port to listen for secondary priority work.");
DEFINE_bool(set_thread_priority, false,
            "Set thread priority with Linux OS, "
            "which requires running with `sudo`.");

namespace {
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 1;
const unsigned kGetRequestsTotal = 2;
const unsigned kPurgeThreadPoolThreads = 2;
const dans::Priority kPrimaryPriority = 0;
const dans::Priority kSecondaryPriority = 1;
}  // namespace

namespace dans_throttling_proxy {

DansThrottlingProxy::DansThrottlingProxy()
    : _jid_factory(0),
      _comm_handler(),
      _exec(kPurgeThreadPoolThreads),
      _proxy(std::vector<unsigned>(kMaxPrio + 1, kThreadsPerPrio),
             FLAGS_set_thread_priority, &_comm_handler) {
  // Start monitoring primary and secondary ports.
  _comm_handler.Serve(FLAGS_primary_prio_port_in,
                      dans::LinuxCommunicationHandler::CallBack1(std::bind(
                          &DansThrottlingProxy::ReceivedConnection, this,
                          kPrimaryPriority, &_comm_handler, &_proxy,
                          &_jid_factory, &_exec, std::placeholders::_1)));
  _comm_handler.Serve(FLAGS_secondary_prio_port_in,
                      dans::LinuxCommunicationHandler::CallBack1(std::bind(
                          &DansThrottlingProxy::ReceivedConnection, this,
                          kSecondaryPriority, &_comm_handler, &_proxy,
                          &_jid_factory, &_exec, std::placeholders::_1)));
}

DansThrottlingProxy::DansThrottlingProxy(int primary_throttle,
                                         int secondary_throttle,
                                         std::chrono::milliseconds time_period,
                                         ThrottlerCallback th_cb)
    : DansThrottlingProxy() {
  _proxy.GetThrottler()->SetThrottle(kPrimaryPriority, primary_throttle);
  _proxy.GetThrottler()->SetThrottle(kSecondaryPriority, secondary_throttle);
  _proxy.GetThrottler()->RunEveryT(time_period, th_cb);
}

ThrottleInterface* DansThrottlingProxy::GetThrottleInterface() {
  return _proxy.GetThrottler();
}

void DansThrottlingProxy::ReceivedConnection(
    unsigned priority, dans::LinuxCommunicationHandler* comm_handler_p,
    dans::DStageProxy* proxy_p, dans::JobIdFactory* jid_factory_p,
    dans::Executor* exec_p, int soc) {
  // Taking this time now instead of using the constructor of the Job as
  // creating a JobId could block.
  std::chrono::high_resolution_clock::time_point start_time =
      std::chrono::high_resolution_clock::now();

  // Create a unique JobId.
  dans::JobId jid = jid_factory_p->CreateJobId();

  auto client_request = std::make_unique<dans::TcpPipe>(soc);
  auto job = std::make_unique<dans::Job<std::unique_ptr<dans::TcpPipe>>>(
      std::move(client_request), jid, priority, /*duplication*/ 0, start_time);

  auto memory_ptr = &(job->job_data->first_cb);
  // Monitor socket for failures and Purge if it is triggered.
  job->job_data->first_cb = comm_handler_p->MonitorNew(
      soc, {false, false},
      [proxy_p, exec_p, jid, priority, memory_ptr](
          int soc, dans::CommunicationHandlerInterface::ReadyFor ready_for) {
        *memory_ptr = nullptr;
        VLOG(1) << "Application purging job_id=" << jid << " prio=" << priority;
        dans::LinuxCommunicationHandler::PrintEpollEvents(ready_for.events);
        exec_p->Submit({[proxy_p, jid]() { proxy_p->Purge(jid); }, jid});
      });

  // Create job and dispatch it.
  VLOG(2) << "Application received connection and created " << job->Describe()
          << " " << job->job_data->Describe();

  proxy_p->Dispatch(std::move(job), /*requested_duplication=*/0);
}

}  // namespace dans_throttling_proxy
