#ifndef DANS02_DSTAGE_APPLICATIONREQUEST_H
#define DANS02_DSTAGE_APPLICATIONREQUEST_H

#include "dstage/applicationrequestdata.h"
#include "dstage/jobid.h"
#include "dstage/destination.h"
#include "dstage/priority.h"

namespace duplicate_aware_scheduling {

class ApplicationRequest {
public:
  ApplicationRequest(unique_ptr<ApplicationRequestData> app_data,
                     const JobId job_id,
                     const DestinationMap destination_map,
                     Priority priority);
  bool operator==(Symbol& rhs) const;

  const JobId GetJobId();

  Destination GetLocation(Priority incoming_priority);

  unique_ptr<ApplicationRequestData> GetApplicationData();

private:
  unique_ptr<ApplicationRequestData> _app_data;
  const JobId job_id;
  const DestinationMap _destination_map;
  Destination _destination;
  Priority _priority;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_APPLICATIONREQUEST_H