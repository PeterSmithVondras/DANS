#ifndef DANS02_COMMON_UTIL_CALLBACK_H
#define DANS02_COMMON_UTIL_CALLBACK_H

#include <functional>
#include "glog/logging.h"

namespace util {
namespace callback {
enum class CallbackDeleteOption { DELETE_AFTER_CALLING, DO_NOTHING };

template <typename... Ts>
class CallbackBase {
 public:
  virtual ~CallbackBase() {}
  virtual void operator()(Ts... args) = 0;
  virtual void Run(Ts... args) = 0;
};

template <typename... Ts>
class Callback : CallbackBase<Ts...> {
 public:
  Callback() = delete;

  Callback(std::function<void(Ts...)>&& cb, CallbackDeleteOption option);

  // If not indication of whether to delete after run is given, object is not
  // deleted.
  Callback(std::function<void(Ts...)>&& cb);

  ~Callback();

  void operator()(Ts... args);

  void Run(Ts... args);

 private:
  std::function<void(Ts...)> _cb;
  const bool _delete_after_run;
  std::mutex lock;
};

}  // namespace callback
}  // namespace util

#include "common/util/callback.hh"

#endif  // DANS02_COMMON_UTIL_CALLBACK_H
