cc_library(
    name = "main",
    srcs = ["lib/x86_64-linux-gnu/libglog.so"],
    hdrs = glob(["include/glog/*.h"]),
    copts = [
        "-Iexternal/glog/include",
    ],
    visibility = ["//visibility:public"],
)