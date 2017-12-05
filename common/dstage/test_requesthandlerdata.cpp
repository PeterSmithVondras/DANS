#include "common/dstage/requesthandlerdata.h"

#include <cstdlib>     // EXIT_SUCCESS and EXIT_FAILURE
namespace {
  using duplicate_aware_scheduling::RequestHandlerData;
  using duplicate_aware_scheduling::RequestData;

  int kFileNumber = 5;

  std::string foo("foo");
  std::string bar("bar");
  std::string bin("bin");
}

int main() {
  bool success = true;
  fprintf(stderr, "test_requessthandlerdata...\n");
  
  std::vector<std::string> ips {foo, bar, bin};

  RequestHandlerData data(ips, kFileNumber);
  if (!(data.ip_addresses == ips) || !(data.file_number == kFileNumber)) {
    fprintf(stderr, "FAIL: RequestHandlerData contructor\n");
    success = false;
  }

  std::unique_ptr<RequestData> one   = data.CreateDuplicate(0);
  // std::unique_ptr<RequestData> two   = data.CreateDuplicate(1);
  // std::unique_ptr<RequestData> three = data.CreateDuplicate(2);

  // if (!(one->ip_addresses[0] == foo)   || !(one->file_number == kFileNumber) ||
  //     !(two->ip_addresses[0] == bar)   || !(two->file_number == kFileNumber) ||
  //     !(three->ip_addresses[0] == bin) ||
  //     !(three->file_number == kFileNumber)) {
  //   fprintf(stderr, "FAIL: RequestHandlerData.CreateDuplicate\n");
  //   success = false;
  // }


  if (success) {
    printf(" Passed\n");
    return 0;
  } else return 1;
}