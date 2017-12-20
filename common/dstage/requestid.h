#ifndef DANS02_DSTAGE_REQUESTID_H
#define DANS02_DSTAGE_REQUESTID_H

namespace duplicate_aware_scheduling {
typedef RequestId const int;

class RequestIdFactory {
 public:
  RequestIdFactory(int seed);
  RequestIdFactory CreateRequestId();

 private:
  int _next_job;
};
}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_REQUESTID_H