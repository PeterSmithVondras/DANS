#include "common/dstage/requesthandlerdata.h"

#include <memory>

namespace duplicate_aware_scheduling {

RequestHandlerData::RequestHandlerData(std::vector<std::string> ip_addresses,
                                       int file_number)
    : ip_addresses(ip_addresses), file_number(file_number) {}

std::unique_ptr<RequestData> RequestHandlerData::CreateDuplicate(
    Priority priority) {
  if (priority >= ip_addresses.size()) {
    fprintf(stderr,
            "ERROR: Attempted to CreateDuplicate of RequestHandlerData "
            "with priority %u when ip_addresses size was only %lu.\n",
            priority, ip_addresses.size());
    exit(1);
  }

  std::vector<std::string> ips{ip_addresses[priority]};
  std::unique_ptr<RequestData> data(new RequestHandlerData(ips, priority));
  return data;
}

}  // namespace duplicate_aware_scheduling
