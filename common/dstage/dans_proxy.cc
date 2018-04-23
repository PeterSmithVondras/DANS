#include <chrono>
#include <csignal>
#include <functional>

#include "common/dstage/executor.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/proxy_dstage.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(server_ip, "192.168.137.127", "ip address of the file server.");
DEFINE_uint64(primary_prio_port_in, 5010,
              "Port to listen for primary priority work.");
DEFINE_uint64(secondary_prio_port_in, 5011,
              "Port to listen for secondary priority work.");
DEFINE_uint64(primary_prio_port_out, 5012,
              "Port to send primary priority work to.");
DEFINE_uint64(secondary_prio_port_out, 5013,
              "Port to send secondary priority work to.");
DEFINE_bool(set_thread_priority, false,
            "Set thread priority with Linux OS, "
            "which requires running with `sudo`.");
DEFINE_int64(run_time, 10,
             "Length of time to run this process. Use -1 for infinite.");

namespace {
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 1;
const unsigned kGetRequestsTotal = 2;
const unsigned kPurgeThreadPoolThreads = 2;
const unsigned kHighPriority = 0;
const unsigned kLowPriority = 1;
}  // namespace

void SigPipeHandler(int signal) { LOG(ERROR) << "Received SIGPIPE"; }

void ReceivedConnection(unsigned priority,
                        dans::LinuxCommunicationHandler* comm_handler_p,
                        dans::DStageProxy* proxy_p,
                        dans::JobIdFactory* jid_factory_p,
                        dans::Executor* exec_p, int soc) {
  // Create a unique JobId.
  dans::JobId jid = jid_factory_p->CreateJobId();
  // Monitor socket for failures and Purge if it is triggered.
  std::function<void()> del = comm_handler_p->MonitorNew(
      soc, {false, false},
      [proxy_p, exec_p, jid, priority](
          int soc, dans::CommunicationHandlerInterface::ReadyFor ready_for) {
        PLOG_IF(WARNING, ready_for.err != 0)
            << "Application (comm_handler) received server socket error";
        dans::LinuxCommunicationHandler::PrintEpollEvents(ready_for.events);
        exec_p->Submit({[proxy_p, jid]() { proxy_p->Purge(jid); }, jid});
      });

  // Create job and dispatch it.
  auto client_request = std::make_unique<dans::TcpPipe>(soc, del);
  auto job = std::make_unique<dans::Job<std::unique_ptr<dans::TcpPipe>>>(
      std::move(client_request), jid, priority, /*duplication*/ 0);
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

  // Install a signal handler
  std::signal(SIGPIPE, SigPipeHandler);

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
