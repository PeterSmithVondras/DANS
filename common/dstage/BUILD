cc_library(
    name = "hello-greet",
    srcs = ["hello-greet.cc"],
    hdrs = ["hello-greet.h"],
)

cc_binary(
    name = "job_test",
    srcs = ["hello-world.cc"],
    deps = [
        ":hello-greet",
        "//glog",
        "//gflags",
    ],
)