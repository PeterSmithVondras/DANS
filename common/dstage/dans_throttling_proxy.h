#ifndef COMMON_DSTAGE_DANS_THROTTLING_PROXY
#define COMMON_DSTAGE_DANS_THROTTLING_PROXY

#include "common/dstage/executor.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/proxy_dstage.h"

namespace dans_throttling_proxy {

class DansThrottlingProxy {
 public:
  DansThrottlingProxy();

  // This object holds state and creates threads that would be impractical to
  // copy around.
  DansThrottlingProxy(const DansThrottlingProxy&) = delete;

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