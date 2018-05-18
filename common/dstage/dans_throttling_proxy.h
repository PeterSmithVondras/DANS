#ifndef COMMON_DSTAGE_DANS_THROTTLING_PROXY
#define COMMON_DSTAGE_DANS_THROTTLING_PROXY

#include "common/dstage/executor.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/priority.h"
#include "common/dstage/proxy_dstage.h"
#include "common/dstage/throttler.h"

namespace dans_throttling_proxy {

// Use a pointer to this to call RunEveryT() or SetThrottle without the
// constructor or to override original values.
using ThrottleInterface = dans::Throttler<std::unique_ptr<dans::TcpPipe>>;
// This object contains all the good info on jobs that completed since the list
// time that your callback was called.
using ThrottleStats = dans::Throttler<std::unique_ptr<dans::TcpPipe>>::Stats;
// Your callback should have this type. Note that you can bind values to it for
// use using std::bind().
using ThrottlerCallback = std::function<void(
    ThrottleInterface*, ThrottleStats&&, std::chrono::milliseconds)>;
// Use these when calling ThrottleInterface::SetThrottle(Priority, int);
const dans::Priority kPrimaryPriority = 0;
const dans::Priority kSecondaryPriority = 1;

class DansThrottlingProxy {
 public:
  DansThrottlingProxy();
  DansThrottlingProxy(int primary_throttle, int secondary_throttle,
                      std::chrono::milliseconds time_period, ThrottlerCallback);

  // This object holds state and creates threads that would be impractical to
  // copy around.
  DansThrottlingProxy(const DansThrottlingProxy&) = delete;

  // Get this interface to call RunEveryT() or SetThrottle() from outside of
  // of your ThrottleCallback.
  ThrottleInterface* GetThrottleInterface();

  // void RunEveryT(
  //     std::chrono::milliseconds time_period_estimate,
  //     ThrottlerCallback cb);
  // void SetThrottle(dans::Priority prio, int threshold);

 private:
  dans::JobIdFactory _jid_factory;
  dans::LinuxCommunicationHandler _comm_handler;
  dans::Executor _exec;
  dans::DStageProxy _proxy;

  void ReceivedConnection(unsigned priority,
                          dans::LinuxCommunicationHandler* comm_handler_p,
                          dans::DStageProxy* proxy_p,
                          dans::JobIdFactory* jid_factory_p,
                          dans::Executor* exec_p, int soc);
};

}  // namespace dans_throttling_proxy

#endif  // COMMON_DSTAGE_DANS_THROTTLING_PROXY