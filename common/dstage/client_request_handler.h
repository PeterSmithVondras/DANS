#ifndef DANS02_CLIENT_REQUEST_HANDLER_H
#define DANS02_CLIENT_REQUEST_HANDLER_H

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>

#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/job_types.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

class RequestDispatcher : public Dispatcher<RequestData, RequestData> {
 public:
  RequestDispatcher(Priority max_priority);

 protected:
  void DuplicateAndEnqueue(UniqJobPtr<RequestData> job_in, Priority max_prio,
                           unsigned duplication) override;
};

class RequestScheduler : public Scheduler<RequestData> {
 public:
  RequestScheduler(std::vector<unsigned> threads_per_prio,
                   bool set_thread_priority,
                   CommunicationHandlerInterface* comm_interface,
                   BaseDStage<ResponseData>* response_dstage);
  ~RequestScheduler();

 protected:
  void StartScheduling(Priority prio) override;

  unsigned Purge(JobId job_id) override;

 private:
  CommunicationHandlerInterface* _comm_interface;
  BaseDStage<ResponseData>* _response_dstage;
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;

  void RequestCallback(SharedJobPtr<RequestData> old_job, int soc,
                       CommunicationHandlerInterface::ReadyFor ready_for);
};

class RequestDStage : public DStage<RequestData, RequestData> {
 public:
  RequestDStage(std::vector<unsigned> threads_per_prio,
                bool set_thread_priority,
                CommunicationHandlerInterface* comm_interface,
                BaseDStage<ResponseData>* response_dstage)
      : DStage<RequestData, RequestData>(
            threads_per_prio.size() - 1,
            std::make_unique<MultiQueue<RequestData>>(threads_per_prio.size() -
                                                      1),
            std::make_unique<RequestDispatcher>(threads_per_prio.size() - 1),
            std::make_unique<RequestScheduler>(
                threads_per_prio, set_thread_priority, comm_interface,
                response_dstage)) {}
};

}  // namespace dans

#endif  // DANS02_CLIENT_REQUEST_HANDLER_H