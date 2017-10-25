#ifndef APPLICATIONREQUEST_H
#define APPLICATIONREQUEST_H

#include "ApplicationRequestData.h"
#include "JobId.h"
#include "Location.h"
#include "Priority.h"

namespace duplicate_aware_scheduling {

class ApplicationRequest {
public:
  ApplicationRequest(unique_ptr<ApplicationRequestData> app_data,
                     const JobId job_id,
                     const LocationMap location_map);
  bool operator==(Symbol& rhs) const;

  const JobId GetJobId();

  Location GetLocation(Priority incoming_priority);

  unique_ptr<ApplicationRequestData> GetApplicationData();

private:
  unique_ptr<ApplicationRequestData> _app_data;
  const JobId job_id;
  const LocationMap location_map;

};
} // namespace duplicate_aware_scheduling

#endif // APPLICATIONREQUEST_H