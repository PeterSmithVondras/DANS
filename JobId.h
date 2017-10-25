#ifndef JOBID_H
#define JOBID_H

namespace duplicate_aware_scheduling {
typedef struct JobId {
  JobId(source, job_number);
  bool operator==(Symbol& rhs) const;

  const int _source;
  const uint _job_number;
} JobId;

} // namespace duplicate_aware_scheduling

#endif // JOBID_H