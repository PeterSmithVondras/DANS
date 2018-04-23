#ifndef DANS02_DSTAGE_SYNCHRONIZATION_H
#define DANS02_DSTAGE_SYNCHRONIZATION_H

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace dans {

// Thread safe object used for tracking if a job is purged.
class PurgeState {
 public:
  PurgeState();
  // Returns whether the this job is marked as purged. There is no guarantee
  // that this is accurate even at the time of return.
  bool IsPurged();
  // If the job is not already purged, this will set it as purged and return
  // true. If another thread sets the the job purged, then it will return false.
  bool SetPurged();

 private:
  // guards state of whether job has been purged.
  std::shared_timed_mutex _state_shared_mutex;
  bool _purged;
};

// Connection class should be broken out into its own header and implementation
// files.
class Connection {
 public:
  // Explicitly deleting default and copy constructor.
  Connection() = delete;
  Connection(const Connection&) = delete;
  Connection(int socket);
  ~Connection();

  int Socket();
  // Shutdown read and write to a socket which will trigger epoll.
  void Shutdown();
  void SetShutdown();
  void Close();
  bool IsClosed();

 private:
  const int _socket;
  std::mutex _close_lock;
  bool _closed;
  bool _shutdown;
};

// Thread safe counter.
class Counter {
 public:
  Counter(int initial_value);
  // Returns new value of Count
  int Increment();
  int Decrement();
  int Count();

 private:
  // guards state of whether job has been purged.
  std::shared_timed_mutex _state_shared_mutex;
  int _count;
};

}  // namespace dans

#endif  // DANS02_DSTAGE_SYNCHRONIZATION_H
