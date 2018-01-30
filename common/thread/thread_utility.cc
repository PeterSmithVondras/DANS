#include <sched.h>
#include <unistd.h>

#include "common/thread/thread_utility.h"
#include "glog/logging.h"

namespace threadutility {

void ThreadUtility::SetPriority(std::thread& thread, int policy, int priority) {
  pthread_t threadID = (pthread_t)thread.native_handle();
  struct sched_param param;
  int old_policy;

  PLOG_IF(ERROR, pthread_getschedparam(threadID, &old_policy, &param) != 0)
      << "pthread_getschedparam failed to get thread policy and parameters in "
      << "threadutulity::ThreadUtility::SetPriority";
  int old_priority = param.sched_priority;
  param.sched_priority = priority;
  PLOG_IF(ERROR, pthread_setschedparam(threadID, policy, &param) != 0)
      << "pthread_setschedparam param failed to set thread policy and "
      << "priority in threadutulity::ThreadUtility::SetPriority "
      << "when attempting to set thread to: (policy=" << PolicyToString(policy)
      << ", priority=" << priority << ")";

  VLOG(1) << "Thread INITIALLY: (policy=" << PolicyToString(old_policy)
          << ", priority=" << old_priority
          << ")   SET TO: (policy=" << PolicyToString(policy)
          << ", priority=" << priority << ")";
}

std::string ThreadUtility::PolicyToString(int policy) {
  return ((policy == SCHED_FIFO)
              ? "SCHED_FIFO"
              : (policy == SCHED_RR)
                    ? "SCHED_RR"
                    : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???");
}

}  // namespace threadutility
