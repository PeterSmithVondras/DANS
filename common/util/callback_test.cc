#include "common/util/callback.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using util::callback::Callback;
using util::callback::CallbackDeleteOption;
const int kInitValue = 0;
const int kEndValue = kInitValue + 5;
void AddOneToFirstIntPointer(int* number, char a, int b) { (*number)++; }
}  // namespace

TEST(CallbackTest, UniquePtr) {
  int x = kInitValue;
  auto cb = new Callback<std::unique_ptr<int*>>(
      [](std::unique_ptr<int*> number) { **number = kEndValue; },
      CallbackDeleteOption::DELETE_AFTER_CALLING);
  cb->Run(std::make_unique<int*>(&x));
  CHECK_EQ(x, kEndValue);
}

TEST(CallbackTest, lvalue) {
  int x = kInitValue;
  auto bar = new Callback<int*>([](int* number) { *number = kEndValue; },
                                CallbackDeleteOption::DELETE_AFTER_CALLING);
  (*bar)(&x);
  CHECK_EQ(x, kEndValue);
}

TEST(CallbackTest, SeveralArgs) {
  int x = kInitValue;
  Callback<int*, char, int> cb(AddOneToFirstIntPointer);
  std::function<void(int*, char, int)> bang(std::move(cb));

  for (int i = 0; i < kEndValue - kInitValue; i++) {
    bang(&x, 'a', 0);
  }

  CHECK_EQ(x, kEndValue);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  return RUN_ALL_TESTS();
}