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
  hdrs = [
    "multiqueue.h",
    "multiqueue.hh",
  ],
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
  hdrs = [
    "dispatcher.h",
    "dispatcher.hh",
  ],
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
  hdrs = [
    "scheduler.h",
    "scheduler.hh",
  ],
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
  hdrs = [
    "dstage.h",
    "dstage.hh"
  ],
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
  name = "client_connect_handler",
  hdrs = [
    "client_connect_handler.h",
  ],
  srcs = [ "client_connect_handler.cc" ],
  deps = [
    ":dstage_base",
    ":job_state",
    ":job_types",
    ":multiqueue",
    ":dispatcher",
    ":scheduler",
    ":dstage",
    ":linux_communication_handler",
    ":client_request_handler",
    "@glog//:main",
    "@gflags//:main",
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
    ":job_state",
    ":job_types",
    ":multiqueue",
    ":dispatcher",
    ":scheduler",
    ":dstage",
    ":linux_communication_handler",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_library(
  name = "client_response_handler",
  hdrs = [
    "client_response_handler.h",
  ],
  srcs = [ "client_response_handler.cc" ],
  deps = [
    ":dstage_base",
    ":job_state",
    ":job_types",
    ":multiqueue",
    ":dispatcher",
    ":scheduler",
    ":dstage",
    ":linux_communication_handler",
    ":client_request_handler",
    "@glog//:main",
    "@gflags//:main",
  ],
)

cc_test(
  name = "file_client_dstage_chain_test",
  srcs = [ "file_client_dstage_chain_test.cc" ],
  deps = [
    ":job_types",
    ":client_connect_handler",
    ":client_request_handler",
    ":client_response_handler",
    ":linux_communication_handler",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "job_state",
  hdrs = [
    "job_state.h",
  ],
  srcs = [ "job_state.cc" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
  ],
)


cc_library(
  name = "job_state_test",
  hdrs = [
    "job_state_test.h",
  ],
  srcs = [ "job_state_test.cc" ],
  deps = [
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)

cc_library(
  name = "job_types",
  hdrs = [
    "job_types.h",
  ],
  deps = [
    "@glog//:main",
  ],
)

