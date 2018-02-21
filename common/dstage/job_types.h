#ifndef DANS02_DSTAGE_JOB_TYPES_H
#define DANS02_DSTAGE_JOB_TYPES_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/dstage/synchronization.h"

namespace dans {

struct ConnectData {
  std::vector<std::string> ip_addresses;
  std::vector<std::string> ports;
  std::shared_ptr<
      std::function<void(unsigned priority, char* response, int length)>>
      done;
};

struct ConnectDataInternal {
  std::string ip;
  std::string port;
  std::shared_ptr<
      std::function<void(unsigned priority, char* response, int length)>>
      done;
  std::shared_ptr<PurgeState> purge_state;
};

struct RequestData {
  int soc;
  std::shared_ptr<
      std::function<void(unsigned priority, char* response, int length)>>
      done;
  std::shared_ptr<PurgeState> purge_state;
};

}  // namespace dans

#endif  // DANS02_DSTAGE_JOB_TYPES_H
