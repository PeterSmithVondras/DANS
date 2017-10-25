#ifndef DSTAGE_H
#define DSTAGE_H

#include <memory>

#include "ApplicationRequest.h"
#include "JobId"
#include "Priority.h"
#include "util/status.h"


namespace duplicate_aware_scheduling {

class DStage {
public:
  virtual Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                          Priority base_prio,
                          uint duplication_level);

  virtual Status Purge(JobId job_id);
};
} // namespace duplicate_aware_scheduling

#endif // DSTAGE_H