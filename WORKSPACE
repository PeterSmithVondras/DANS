
#Using web uri
#new_http_archive(
#    name = "gtest",
#    url = "https://github.com/google/googletest/archive/release-1.7.0.zip",
#    sha256 = "b58cb7547a28b2c718d1e38aee18a3659c9e3ff52440297e965f5edffe34b6d0",
#    build_file = "gtest.BUILD",
#    strip_prefix = "googletest-release-1.7.0",
#)

# gtest using ubuntu package
new_local_repository(
     name = "gtest",
     path = "/usr/src/googletest/googletest",
     build_file = "gtest.BUILD",
)

# gflags using ubuntu package
new_local_repository(
     name = "gflags",
     path = "/usr",
     build_file = "gflags.BUILD",
)

# glog using ubuntu package
new_local_repository(
     name = "glog",
     path = "/usr",
     build_file = "glog.BUILD",
)







