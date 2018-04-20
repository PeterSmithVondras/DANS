#ifndef DANS02_COMMUNICATION_HANDLER_INTERFACE_H
#define DANS02_COMMUNICATION_HANDLER_INTERFACE_H

#include <functional>
#include <string>

namespace dans {

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
  using Deleter = std::function<void()>;

  virtual ~CommunicationHandlerInterface() {}

  virtual int CreateSocket() = 0;

  virtual void Serve(unsigned short port, CallBack1 done) = 0;

  virtual Deleter Connect(const std::string& ip, const std::string& port,
                          CallBack2 done) = 0;

  virtual Deleter Monitor(int soc, ReadyFor ready_for, CallBack2 done) = 0;

  virtual Deleter MonitorNew(int soc, ReadyFor ready_for, CallBack2 done) = 0;

  virtual void Close(int soc) = 0;
};
}  // namespace dans

#endif  // DANS02_COMMUNICATION_HANDLER_INTERFACE_H