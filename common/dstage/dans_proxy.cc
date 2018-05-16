#include <chrono>
#include <csignal>
#include <functional>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>

#include "common/dstage/executor.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/proxy_dstage.h"
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
DEFINE_int64(run_time, -1,
             "Length of time to run this process. Use -1 for infinite.");

DEFINE_uint64(tos_high_prio, 4,
              "TOS value for high priority traffic");

DEFINE_uint64(tos_low_prio, 8,
              "TOS value for low priority traffic");
DEFINE_bool(set_network_priority, false,
            "Set network priorioritzation");


namespace {
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 1;
const unsigned kGetRequestsTotal = 2;
const unsigned kPurgeThreadPoolThreads = 2;
const unsigned kHighPriority = 0;
const unsigned kLowPriority = 1;
}  // namespace

void ReceivedConnection(unsigned priority,
                        dans::LinuxCommunicationHandler* comm_handler_p,
                        dans::DStageProxy* proxy_p,
                        dans::JobIdFactory* jid_factory_p,
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

  VLOG(3) << "XXX:I am here - val of network prio = " << FLAGS_set_network_priority << "  " << FLAGS_tos_high_prio << "  " << FLAGS_tos_low_prio;
  if (FLAGS_set_network_priority) {
    
     unsigned int net_prio = 0;
     if (priority == 0)
	net_prio = FLAGS_tos_high_prio;
     else
	net_prio = FLAGS_tos_low_prio;

     if (setsockopt(soc, IPPROTO_IP, IP_TOS, &net_prio, sizeof(net_prio)) < 0)
        VLOG(0) << "XXX: Error in setting  IP_TOS option:" << net_prio; 
     else
	VLOG(3) << "Setting TOS bit = " << net_prio << "for job id = " << jid;	 
    
     } 


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

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  dans::LinuxCommunicationHandler comm_handler;
  dans::DStageProxy proxy(std::vector<unsigned>(kMaxPrio + 1, kThreadsPerPrio),
                          FLAGS_set_thread_priority, &comm_handler);
  dans::JobIdFactory jid_factory(0);

  // Creating threadpool for purging as Purge is a blocking call.
  dans::Executor exec(kPurgeThreadPoolThreads);

  // Start monitoring primary and secondary ports.
  comm_handler.Serve(FLAGS_primary_prio_port_in,
                     dans::LinuxCommunicationHandler::CallBack1(std::bind(
                         &ReceivedConnection, kHighPriority, &comm_handler,
                         &proxy, &jid_factory, &exec, std::placeholders::_1)));
  comm_handler.Serve(FLAGS_secondary_prio_port_in,
                     dans::LinuxCommunicationHandler::CallBack1(std::bind(
                         &ReceivedConnection, kLowPriority, &comm_handler,
                         &proxy, &jid_factory, &exec, std::placeholders::_1)));

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
