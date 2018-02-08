#ifndef DANS02_CLIENT_REQUEST_HANDLER_H
#define DANS02_CLIENT_REQUEST_HANDLER_H

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>

#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

struct ReqData {
  std::vector<std::string> ip_addresses;
  std::vector<std::string> ports;
  std::function<void(int)> done;
};

struct ReqDataInternal {
  int soc;
  std::function<void(int)> done;
};

class RequestDispatcher : public Dispatcher<ReqData, ReqDataInternal> {
 public:
  RequestDispatcher(Priority max_priority);

 protected:
  void DuplicateAndEnqueue(UniqConstJobPtr<ReqData> job_in, Priority max_prio,
                           unsigned duplication) override;
};

class RequestScheduler : public Scheduler<ReqDataInternal> {
 public:
  RequestScheduler(std::vector<unsigned> threads_per_prio);
  ~RequestScheduler();

 protected:
  void StartScheduling(Priority prio) override;

 private:
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;
};

}  // namespace dans

#endif  // DANS02_CLIENT_REQUEST_HANDLER_H