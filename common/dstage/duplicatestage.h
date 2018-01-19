#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <memory>
#include <mutex>

#include "common/dstage/dispatcher.h"
// #include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace dans {

template <typename T>
class DuplicateStage /*: public DStage<T>*/ {
 public:
  DuplicateStage(Priority max_priority,
                 std::unique_ptr<Dispatcher<T>> dispatcher);
  ~DuplicateStage() {}

 protected:
  const Priority _max_priority;
  MultiQueue<T> _multi_q;
  std::unique_ptr<Dispatcher<T>> _dispatcher;
  // std::unique_ptr<scheduler<T>> _scheduler;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DUPLICATESTAGE_H