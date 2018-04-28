#ifndef DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH
#define DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH

#include "common/util/callback.h"
#include "glog/logging.h"

namespace util {
namespace callback {

template <typename... Ts>
Callback<Ts...>::Callback(std::function<void(Ts...)>&& cb,
                          CallbackDeleteOption option)
    : _cb(std::move(cb)),
      _delete_after_run(
          option == CallbackDeleteOption::DELETE_AFTER_CALLING ? true : false) {
}

template <typename... Ts>
Callback<Ts...>::Callback(std::function<void(Ts...)>&& cb)
    : _cb(std::move(cb)), _delete_after_run(false) {}

template <typename... Ts>
Callback<Ts...>::~Callback() {
  lock.lock();
}

template <typename... Ts>
void Callback<Ts...>::operator()(Ts... args) {
  Run(std::move(args)...);
}

template <typename... Ts>
void Callback<Ts...>::Run(Ts... args) {
  lock.lock();
  _cb(std::move(args)...);
  lock.unlock();
  if (_delete_after_run) {
    delete this;
  }
}

}  // namespace callback
}  // namespace util

#endif  // DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH
