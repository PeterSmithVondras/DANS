#include "common/dstage/proxy_dstage.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
}  // namespace

namespace dans {

ProxyScheduler::ProxyScheduler(std::vector<unsigned> threads_per_prio,
                               bool set_thread_priority,
                               CommunicationHandlerInterface* comm_interface)
    : Scheduler<ClientConnection>(threads_per_prio, set_thread_priority),
      _comm_interface(comm_interface),
      _destructing(false) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

ProxyScheduler::~ProxyScheduler() {
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

}  // namespace dans