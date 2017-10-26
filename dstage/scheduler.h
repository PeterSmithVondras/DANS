#ifndef DANS02_DSTAGE_SCHEDULER_H
#define DANS02_DSTAGE_SCHEDULER_H

#include "dstage/dstage.h"
#include "dstage/dispatcher.h"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class Scheduler : DStage {
public:

  // Points the Scheduler to a specific Dispatcher which may now request work.
  Status LinkToDispatcher(Dispatcher* dispatcher);

protected:
  Dispatcher* _dispatcher;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_SCHEDULERCHER_H