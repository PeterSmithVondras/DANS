cc_library(
  name = "dstage_base",
  hdrs = [
    "job.h",
    "priority.h",
    "basemultiqueue.h",
    "basedispatcher.h",
    "basescheduler.h",
    "basedstage.h",
  ],
)

cc_test(
  name = "job_test",
  size = "small",
  srcs = [ "job_test.cc" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "multiqueue",
  hdrs = [ "multiqueue.h" ],
  srcs = [ "multiqueue.cc" ],
  linkopts = [ "-lpthread" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "multiqueue_test",
  size = "small",
  srcs = [ "multiqueue_test.cc" ],
  deps = [
    ":dstage_base",
    ":multiqueue",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "dispatcher",
  hdrs = [ "dispatcher.h" ],
  srcs = [ "dispatcher.cc" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "dispatcher_test",
  size = "small",
  srcs = [ "dispatcher_test.cc" ],
  deps = [
    ":dstage_base",
    ":dispatcher",
    ":multiqueue",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "scheduler",
  hdrs = [ "scheduler.h" ],
  srcs = [ "scheduler.cc" ],
  deps = [
    ":dstage_base",
    "//common/thread:thread_utility",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "scheduler_test",
  size = "small",
  srcs = [ "scheduler_test.cc" ],
  deps = [
    ":dstage_base",
    ":scheduler",
    ":multiqueue",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "dstage",
  hdrs = [ "dstage.h" ],
  srcs = [ "dstage.cc" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "dstage_test",
  size = "small",
  srcs = [ "dstage_test.cc" ],
  deps = [
    ":dstage",
    ":dstage_base",
    ":dispatcher",
    ":scheduler",
    ":multiqueue",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "linux_communication_handler",
  hdrs = [
    "linux_communication_handler.h",
    "communication_handler_interface.h",
  ],
  srcs = [ "linux_communication_handler.cc" ],
  deps = [
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "linux_communication_handler_test",
  srcs = [ "linux_communication_handler_test.cc" ],
  deps = [
    ":linux_communication_handler",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "client_request_handler",
  hdrs = [
    "client_request_handler.h",
  ],
  srcs = [ "client_request_handler.cc" ],
  deps = [
    ":dstage_base",
    ":multiqueue",
    ":dispatcher",
    ":scheduler",
    ":dstage",
    "@glog//:main",
    "@gflags//:main",
  ],







)