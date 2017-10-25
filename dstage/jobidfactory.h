#ifndef JOBIDFACTORY_H
#define JOBIDFACTORY_H

namespace duplicate_aware_scheduling {
class JobIdFactory {
  public:
    JobIdFactory(int source, int seed);
    JobIdFactory CreateJobId();

  private:
    const int _source;
    int _next_job;
};

} // namespace duplicate_aware_scheduling

#endif // JOBIDFACTORY_H