#ifndef DANS02_DSTAGE_REQUESTHANDLERDATA_H
#define DANS02_DSTAGE_REQUESTHANDLERDATA_H

#include "dstage/requestdata.h"

namespace duplicate_aware_scheduling {

class RequestHandlerData : RequestData {
 public:
  RequestHandlerData(std::vector<string> ip_addresses, int file_number);
  const unique_ptr<std::vector<const string>> ip_addresses;
  const int file_number;
};

} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_REQUESTHANDLERDATA_H