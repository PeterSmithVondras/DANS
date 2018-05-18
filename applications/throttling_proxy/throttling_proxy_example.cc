#include <chrono>
#include <csignal>
#include <functional>

#include "common/dstage/dans_throttling_proxy.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int64(run_time, -1,
             "Length of time to run this process. Use -1 for infinite.");

namespace {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using namespace dans_throttling_proxy;
// This is set to one minute.
const std::chrono::milliseconds kTimePeriodEstimate(60000);
const int kGenericThrottleValue = 1000;

// ***************************************************************************
// Define this function for yourself with awesome useful functionality.
void RunThisFunctionEveryT(bool* your_persistant_state, ThrottleInterface* th,
                           ThrottleStats&& state,
                           std::chrono::milliseconds time) {
  if (state.primary_latencies.size() > 0)
    LOG(INFO) << "RUNEVERYT -- "
              << "Primary count: " << state.primary_latencies.size()
              << ", lat:" << state.primary_latencies[0].count()
              << ", size: " << state.primary_sizes[0];
  if (*your_persistant_state == true) {
    LOG(INFO) << "Setting Trottle to primary=1 secondary=1";
    th->SetThrottle(kPrimaryPriority, 1);
    th->SetThrottle(kSecondaryPriority, 1);
    *your_persistant_state = false;
  } else {
    LOG(INFO) << "Setting Trottle to primary=1000 secondary=1000";
    th->SetThrottle(kPrimaryPriority, 1000);
    th->SetThrottle(kSecondaryPriority, 1000);
    *your_persistant_state = true;
  }
}
}  // namespace

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  // Here is an example of how to keep some state with every run of your
  // callback.
  bool your_persistant_state = false;
  auto callback =
      std::bind(RunThisFunctionEveryT, &your_persistant_state, _1, _2, _3);

  // ***************************************************************************
  // Call this constructor if you would like to use the
  // --primary_priority_throttle and --secondary_priority_throttle flags.
  // Note that you can still control this later by calling
  // GetThrottleInterface().
  // auto dans_throttling_proxy = std::make_unique<DansThrottlingProxy>();

  // ***************************************************************************
  // Call this constructor if you would like to pass in your own throttles and
  // callback function.
  auto dans_throttling_proxy = std::make_unique<DansThrottlingProxy>(
      kGenericThrottleValue, kGenericThrottleValue, kTimePeriodEstimate,
      callback);

  ThrottleInterface* throttler = dans_throttling_proxy->GetThrottleInterface();
  // You can now call RunEveryT() or SetThrottle manually here if you want to up
  // date it outside of the constructor and original RunEveryT callback.
  // throttler->RunEveryT(kTimePeriodEstimate, callback)
  // throttler->SetThrottle(kPrimaryPriority, kGenericThrottleValue);
  // throttler->SetThrottle(kSecondaryPriority, kGenericThrottleValue);

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
