#ifndef DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH
#define DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH

#include "common/util/callback.h"
#include "glog/logging.h"

namespace util {

template <typename... Ts>
Callback<Ts...>::Callback(std::function<void(Ts...)>&& cb, DeleteOption option)
    : _cb(std::move(cb)),
      _delete_after_run(option == DeleteOption::DELETE_AFTER_CALLING ? true
                                                                     : false) {}

template <typename... Ts>
Callback<Ts...>::Callback(std::function<void(Ts...)>&& cb)
    : _cb(std::move(cb)), _delete_after_run(false) {}

template <typename... Ts>
Callback<Ts...>::~Callback() {}

template <typename... Ts>
void Callback<Ts...>::operator()(Ts... args) {
  Run(std::move(args)...);
}

template <typename... Ts>
void Callback<Ts...>::Run(Ts... args) {
  _cb(std::move(args)...);
  if (_delete_after_run) {
    VLOG(0) << "deleting this";
    delete this;
  }
}

}  // namespace util

#endif  // DANS02_COMMON_UTIL_CALLBACK_IMPLEMENTATION_HH
