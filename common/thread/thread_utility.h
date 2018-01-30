#ifndef DANS02_THREAD_UTILITY_H
#define DANS02_THREAD_UTILITY_H

#include <string>
#include <thread>

namespace threadutility {
class ThreadUtility {
 public:
  static void SetPriority(std::thread& thread, int policy, int priority);
  static std::string PolicyToString(int policy);

 private:
  ThreadUtility(){};
  ~ThreadUtility(){};
};

}  // namespace threadutility

#endif  // DANS02_THREAD_UTILITY_H