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
  ],
)

cc_library(
  name =       "multiqueue",
  hdrs =     [ "multiqueue.h" ],
  srcs =     [ "multiqueue.cc" ],
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
  ],
)

cc_library(
  name =       "dispatcher",
  hdrs =     [ "dispatcher.h" ],
  srcs =     [ "dispatcher.cc" ],
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
  ],
)

cc_library(
  name =       "scheduler",
  hdrs =     [ "scheduler.h" ],
  srcs =     [ "scheduler.cc" ],
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
  ],
)

cc_library(
  name =       "dstage",
  hdrs =     [ "dstage.h" ],
  srcs =     [ "dstage.cc" ],
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
  ],
)