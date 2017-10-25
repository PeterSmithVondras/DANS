#ifndef JOBMAP_H
#define JOBMAP_H

namespace duplicate_aware_scheduling {
typedef struct JobMap {
  JobMap(unique_ptr<ApplicationRequest> app_req,
         unique_ptr<Vector<JobMap**>> instances_of_this_job);

  unique_ptr<ApplicationRequest> _app_req;
  unique_ptr<Vector<JobMap**>> _instances_of_this_job;
} JobId;

} // namespace duplicate_aware_scheduling

#endif // JOBMAP_H