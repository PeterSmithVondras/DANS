cc_library(
    name = "main",
    srcs = [
    	"lib/x86_64-linux-gnu/libboost_date_time.so.1.62.0",
    	"lib/x86_64-linux-gnu/libboost_filesystem.so.1.62.0",
    	"lib/x86_64-linux-gnu/libboost_iostreams.so.1.62.0",
    	"lib/x86_64-linux-gnu/libboost_system.so.1.62.0",
    	"lib/x86_64-linux-gnu/libboost_thread.so.1.62.0"
    ],
    hdrs = glob(["include/boost/*.hpp"]),
    copts = [
        "-Iexternal/boost/include",
    ],
    visibility = ["//visibility:public"],
)