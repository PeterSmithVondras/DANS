#ifndef DANS02_EXECUTOR_H
#define DANS02_EXECUTOR_H

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>

#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/forwarding_dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace {
const int kMaxPrioDansExecutor = 0;
}

namespace dans {
using Closure = std::function<void()>;
struct ExecutorCallback {
  Closure closure;
  unsigned id;
};

class Executor : private DStage<ExecutorCallback, ExecutorCallback> {
 public:
  Executor(unsigned threadpool_size)
      : DStage<ExecutorCallback, ExecutorCallback>(
            kMaxPrioDansExecutor,
            std::make_unique<MultiQueue<ExecutorCallback>>(
                kMaxPrioDansExecutor),
            std::make_unique<ForwardingDispatcher<ExecutorCallback>>(
                kMaxPrioDansExecutor),
            std::make_unique<ExecutorScheduler>(
                std::vector<unsigned>({threadpool_size}),
                /*set_thread_priority=*/false)),
        _jid_factory(1) {}

  void Submit(ExecutorCallback task);

 private:
  class ExecutorScheduler : public Scheduler<ExecutorCallback> {
   public:
    ExecutorScheduler(std::vector<unsigned> threads_per_prio,
                      bool set_thread_priority);
    ~ExecutorScheduler();

   protected:
    void StartScheduling(Priority prio) override;

   private:
    bool _destructing;
    std::shared_timed_mutex _destructing_lock;
  };

  JobIdFactory _jid_factory;
};

}  // namespace dans

#endif  // DANS02_EXECUTOR_H