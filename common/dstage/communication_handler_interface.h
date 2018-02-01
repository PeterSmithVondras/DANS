#ifndef DANS02_COMMUNICATION_HANDLER_INTERFACE_H
#define DANS02_COMMUNICATION_HANDLER_INTERFACE_H

#include <functional>

namespace dans {

class CommunicationHandlerInterface {
 public:
  virtual ~CommunicationHandlerInterface() {}

  virtual int CreateSocket() = 0;
  virtual void Connect() = 0;
};
}  // namespace dans

#endif  // DANS02_COMMUNICATION_HANDLER_INTERFACE_H