#ifndef DANS02_DSTAGE_JOBID_H
#define DANS02_DSTAGE_JOBID_H

namespace duplicate_aware_scheduling {
typedef struct JobId {
  JobId(source, job_number);
  bool operator==(Symbol& rhs) const;

  const int _source;
  const uint _job_number;
} JobId;

class JobIdFactory {
  public:
    JobIdFactory(int source, int seed);
    JobIdFactory CreateJobId();

  private:
    const int _source;
    int _next_job;
};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_JOBID_H