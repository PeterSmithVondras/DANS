#ifndef DANS02_DSTAGE_JOB_TYPES_H
#define DANS02_DSTAGE_JOB_TYPES_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace dans {

struct ConnectData {
  std::vector<std::string> ip_addresses;
  std::vector<std::string> ports;
  std::shared_ptr<std::function<void(int)>> done;
};

struct ConnectDataInternal {
  std::string ip;
  std::string port;
  std::shared_ptr<std::function<void(int)>> done;
};

struct RequestData {
  std::shared_ptr<std::function<void(int)>> done;
  int soc;
};

}  // namespace dans

#endif  // DANS02_DSTAGE_JOB_TYPES_H
