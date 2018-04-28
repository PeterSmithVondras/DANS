#ifndef DANS02_COMMUNICATION_HANDLER_INTERFACE_H
#define DANS02_COMMUNICATION_HANDLER_INTERFACE_H

#include <functional>
#include <string>

#include "common/util/callback.h"

namespace dans {

using DynamicallyAllocatedCallback = util::callback::Callback<uint32_t>;
using UniqCbp = std::unique_ptr<DynamicallyAllocatedCallback>;
using SharedCbp = std::shared_ptr<DynamicallyAllocatedCallback>;
using util::callback::CallbackDeleteOption;

class CommunicationHandlerInterface {
 public:
  struct ReadyFor {
    bool in;
    bool out;
    uint32_t events;
    int err;
  };
  using CallBack2 = std::function<void(int soc, ReadyFor ready_for)>;
  using CallBack1 = std::function<void(int soc)>;

  virtual ~CommunicationHandlerInterface() {}

  virtual int CreateSocket() = 0;

  virtual void Serve(unsigned short port, CallBack1 done) = 0;

  virtual DynamicallyAllocatedCallback* Connect(const std::string& ip,
                                                const std::string& port,
                                                CallBack2 done) = 0;

  virtual DynamicallyAllocatedCallback* Monitor(int soc, ReadyFor ready_for,
                                                CallBack2 done) = 0;

  virtual DynamicallyAllocatedCallback* MonitorNew(int soc, ReadyFor ready_for,
                                                   CallBack2 done) = 0;

  virtual UniqCbp Connect2(const std::string& ip, const std::string& port,
                           CallBack2 done,
                           CallbackDeleteOption delete_option) = 0;
  virtual UniqCbp Monitor2(int soc, ReadyFor ready_for, CallBack2 done,
                           CallbackDeleteOption delete_option) = 0;
  virtual UniqCbp MonitorNew2(int soc, ReadyFor ready_for, CallBack2 done,
                              CallbackDeleteOption delete_option) = 0;

  virtual void Close(int soc) = 0;
};
}  // namespace dans

#endif  // DANS02_COMMUNICATION_HANDLER_INTERFACE_H