cc_library(
  name = "callback",
  hdrs = [
    "callback.h",
    "callback.hh",
  ],
  deps = [
    "@glog//:main",
  ],
  visibility = ["//visibility:public",],
)

cc_test(
  name = "callback_test",
  size = "small",
  srcs = [ "callback_test.cc" ],
  deps = [
    ":callback",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)