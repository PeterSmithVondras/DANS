#include <sys/socket.h>

#include "common/dstage/synchronization.h"

#include "glog/logging.h"

namespace dans {

/****************************  PurgeState  ****************************/

PurgeState::PurgeState() : _purged(false) {}

bool PurgeState::IsPurged() {
  std::shared_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  bool purged = _purged;
  return purged;
}

bool PurgeState::SetPurged() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  bool set = !_purged;
  _purged = true;
  return set;
}

/*****************************  Connection  ****************************/
Connection::Connection(int socket)
    : _socket(socket), _closed(false), _shutdown(false) {}
Connection::~Connection() { Close(); }

int Connection::Socket() { return _socket; }

void Connection::Shutdown() {
  std::lock_guard<std::mutex> lock(_close_lock);
  if (!_closed && !_shutdown) {
    VLOG(3) << "Shutdown socket=" << _socket;
    if (shutdown(_socket, SHUT_RDWR) != 0) {
      PLOG(WARNING) << "Failed to shutdown socket=" << _socket;
    }
    _shutdown = true;
  }
}

void Connection::SetShutdown() {
  std::lock_guard<std::mutex> lock(_close_lock);
  _shutdown = true;
}

void Connection::Close() {
  std::lock_guard<std::mutex> lock(_close_lock);
  if (!_closed) {
    VLOG(2) << "Closed socket=" << _socket;
    if (close(_socket) != 0) {
      PLOG(WARNING) << "Failed to close socket=" << _socket;
    }
    _closed = true;
  }
}
bool Connection::IsClosed() { return _closed; }

/*****************************  Counter  ******************************/
Counter::Counter(int initial_value) : _count(initial_value) {}

int Counter::Increment() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  _count++;
  return _count;
}

int Counter::Decrement() {
  std::unique_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  _count--;
  return _count;
}

int Counter::Count() {
  std::shared_lock<std::shared_timed_mutex> lock(_state_shared_mutex);
  int count = _count;
  return count;
}

}  // namespace dans
