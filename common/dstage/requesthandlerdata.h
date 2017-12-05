#ifndef DANS02_DSTAGE_REQUESTHANDLERDATA_H
#define DANS02_DSTAGE_REQUESTHANDLERDATA_H

#include <string>
#include <vector>

namespace duplicate_aware_scheduling {

struct FileRequestData {
  FileRequestData(std::vector<std::string> ip_addresses,
                     int file_number);
  std::vector<std::string> ip_addresses;
  const int file_number;
};

} // namespace duplicate_aware_scheduling 

#endif // DANS02_DSTAGE_REQUESTHANDLERDATA_H