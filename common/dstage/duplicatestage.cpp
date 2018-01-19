#include "common/dstage/duplicatestage.h"

#include "common/dstage/dstage.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
DuplicateStage<T>::DuplicateStage(Priority max_priority,
                                  std::unique_ptr<Dispatcher<T>> dispatcher)
  : _max_priority(max_priority),
    _multi_q(_max_priority),
    _dispatcher(std::move(dispatcher)) {
  _dispatcher->LinkMultiQ(&_multi_q);
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class DuplicateStage<JData>;

}  // namespace dans