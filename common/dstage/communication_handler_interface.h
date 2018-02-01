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
  };
  using CallBack2 = std::function<void(int soc, ReadyFor ready_for)>;

  virtual ~CommunicationHandlerInterface() {}

  virtual int CreateSocket() = 0;

  virtual void Connect(const std::string& ip, const std::string& port,
                       CallBack2 done) = 0;
  virtual void Monitor(int soc, ReadyFor ready_for, CallBack2 done) = 0;
  virtual void Close(int soc) = 0;
};
}  // namespace dans

#endif  // DANS02_COMMUNICATION_HANDLER_INTERFACE_H