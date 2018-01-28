cc_library(
    name = "main",
    srcs = ["lib/x86_64-linux-gnu/libgflags.so"],
    hdrs = glob(["include/gflags/*.h"]),
    copts = [
        "-Iexternal/gflags/include",
    ],
    visibility = ["//visibility:public"],
)