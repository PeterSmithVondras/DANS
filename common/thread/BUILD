cc_library(
  name = "thread_utility",
  srcs = ["thread_utility.cc"],
  hdrs = ["thread_utility.h"],
  deps = [
      "@glog//:main",
      "@gflags//:main",
  ],
  visibility = ["//common/dstage:__pkg__",],
)

cc_test(
  name = "thread_utility_test",
  srcs = ["thread_utility_test.cc"],
  size = "small",
  deps = [
    ":thread_utility",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)