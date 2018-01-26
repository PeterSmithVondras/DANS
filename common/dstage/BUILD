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
  srcs = [ "job_test.cc" ],
  deps = [
    ":dstage_base",
    "//glog",
    "//gflags",
  ],
)

cc_library(
  name =       "multiqueue",
  hdrs =     [ "multiqueue.h" ],
  srcs =     [ "multiqueue.cc" ],
  linkopts = [ "-lpthread" ],
  deps = [
    ":dstage_base",
    "//glog",
    "//gflags",
  ],
)

cc_test(
  name = "multiqueue_test",
  srcs = [ "multiqueue_test.cc" ],
  deps = [
    ":dstage_base",
    ":multiqueue",
    "//glog",
    "//gflags",
  ],
)

cc_library(
  name =       "dispatcher",
  hdrs =     [ "dispatcher.h" ],
  srcs =     [ "dispatcher.cc" ],
  deps = [
    ":dstage_base",
    "//glog",
    "//gflags",
  ],
)

cc_test(
  name = "dispatcher_test",
  srcs = [ "dispatcher_test.cc" ],
  deps = [
    ":dstage_base",
    ":dispatcher",
    ":multiqueue",
    "//glog",
    "//gflags",
  ],
)

cc_library(
  name =       "scheduler",
  hdrs =     [ "scheduler.h" ],
  srcs =     [ "scheduler.cc" ],
  deps = [
    ":dstage_base",
    "//glog",
    "//gflags",
  ],
)

cc_test(
  name = "scheduler_test",
  srcs = [ "scheduler_test.cc" ],
  deps = [
    ":dstage_base",
    ":scheduler",
    ":multiqueue",
    "//glog",
    "//gflags",
  ],
)

cc_library(
  name =       "dstage",
  hdrs =     [ "dstage.h" ],
  srcs =     [ "dstage.cc" ],
  deps = [
    ":dstage_base",
    "//glog",
    "//gflags",
  ],
)

cc_test(
  name = "dstage_test",
  srcs = [ "dstage_test.cc" ],
  deps = [
    ":dstage",
    ":dstage_base",
    ":dispatcher",
    ":scheduler",
    ":multiqueue",
    "//glog",
    "//gflags",
  ],
)