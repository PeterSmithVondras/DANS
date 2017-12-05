#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <memory>
#include <vector>

#include "dstage/dispatcher.h"
#include "dstage/dstage.h"
#include "dstage/request.h"
#include "dstage/scheduler.h"

namespace duplicate_aware_scheduling {
template <class T>
class DuplicateStage : public DStage {
public:
  DuplicateStage(unique_ptr<Dispatcher> _dispatcher,
                 unique_ptr<Scheduler> _scheduler);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  bool Dispatch(Job<T> app_req) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  bool Purge(JobId request_id) override;

private:
  std::unique_ptr<Dispatcher<T>> _dispatcher;
  std::unique_ptr<Scheduler<T>>  _scheduler;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DUPLICATESTAGE_H