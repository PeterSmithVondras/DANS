#include "common/hello_world/hello-greet.h"
#include "gtest/gtest.h"

TEST(HelloTest, GetGreet) { EXPECT_EQ(get_greet("Bazel"), "Hello Bazel"); }