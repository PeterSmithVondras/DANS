#ifndef DANS02_DSTAGE_APPLICATIONREQUEST_H
#define DANS02_DSTAGE_APPLICATIONREQUEST_H

#include "dstage/applicationrequestdata.h"
#include "dstage/jobid.h"
#include "dstage/destination.h"
#include "dstage/priority.h"
#include "dstage/state.h"

namespace duplicate_aware_scheduling {

class ApplicationRequest {
public:
  ApplicationRequest(unique_ptr<ApplicationRequestData> app_data,
                     const JobId job_id,
                     const Destination destination,
                     Priority priority);

  // Tests that two ApplicationRequests have the same JobId.
  bool operator==(Symbol& rhs) const;

  const JobId GetJobId();

  Destination GetLocation(Priority incoming_priority);

  unique_ptr<ApplicationRequestData> GetApplicationData();

  Priority GetPriority();

  State getState();

  Status SetState();

private:
  unique_ptr<ApplicationRequestData> _app_data; 
  const JobId job_id;
  Destination _destination;
  Priority _priority;
  State _state;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_APPLICATIONREQUEST_H