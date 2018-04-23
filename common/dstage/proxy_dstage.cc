#include <sys/socket.h>
#include <sys/types.h>

#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/proxy_dstage.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
const std::string kLowPrioPort = "5012";
const std::string kHighPrioPort = "5013";
const std::string ip = "192.168.137.127";
const int kBufSize = 4096;
const int kWorkerThreadpoolSize = 2;
}  // namespace

namespace dans {

std::string TcpPipe::Describe() {
  std::ostringstream description;
  description << "client_socket=";
  if (in != nullptr) {
    description << (in->connection != nullptr
                        ? std::to_string(in->connection->Socket())
                        : "?");
  } else {
    description << "?";
  }

  description << " server_socket=";

  if (out != nullptr) {
    description << (out->connection != nullptr
                        ? std::to_string(out->connection->Socket())
                        : "?");
  } else {
    description << "?";
  }
  return description.str();
}

std::string TcpPipe::Which(int soc) {
  if (in != nullptr) {
    if (in->connection != nullptr) {
      if (in->connection->Socket() == soc) {
        return "client";
      }
    }
  }

  if (out != nullptr) {
    if (out->connection != nullptr) {
      if (out->connection->Socket() == soc) {
        return "server";
      }
    }
  }

  return "?";
}

void TcpPipe::ShutdownOther(int soc) {
  if (in != nullptr) {
    if (in->connection != nullptr) {
      if (in->connection->Socket() != soc) {
        out->connection->SetShutdown();
        in->connection->Shutdown();
        return;
      }
    }
  }

  if (out != nullptr) {
    if (out->connection != nullptr) {
      if (out->connection->Socket() != soc) {
        in->connection->SetShutdown();
        out->connection->Shutdown();
        return;
      }
    }
  }
}

int TcpPipe::OtherSocket(int soc) {
  int other;
  if (in != nullptr) {
    if (in->connection != nullptr) {
      other = in->connection->Socket();
      if (other != soc) {
        return other;
      }
    }
  }

  if (out != nullptr) {
    if (out->connection != nullptr) {
      other = out->connection->Socket();
      if (other != soc) {
        return other;
      }
    }
  }

  return -1;
}

int TcpPipe::Pipe(int read_soc) {
  int send_soc = OtherSocket(read_soc);
  int total_bytes_piped = 0;
  int bytes_read = 0;
  int bytes_sent = 0;
  int bytes_sent_this_round = 0;
  char buf[kBufSize];

  while (true) {
    // Read the read_socket
    while (true) {
      bytes_read = read(read_soc, buf, kBufSize);
      if (bytes_read == -1) {
        if (errno == EAGAIN) {
          return total_bytes_piped == 0 ? -TRY_LATER : total_bytes_piped;
        } else if (errno == EWOULDBLOCK) {
          break;
        } else {
          return -READ_FAIL;
        }
      } else if (bytes_read == 0) {
        // Reading socket connection closed by peer.
        // TODO: This might be a lie because we could have already sent some
        // bytes but we want to signal socket closed.
        return 0;
      } else {
        // Bytes were read so we should send them.
        break;
      }
    }

    // If we get out of read loop and bytes_read is -1 then we are done.
    if (bytes_read == -1) {
      return total_bytes_piped;
    }

    // Send the bytes_read
    bytes_sent_this_round = 0;
    while (true) {
      bytes_sent =
          send(send_soc, buf + bytes_sent_this_round, bytes_read, /*flags=*/0);
      if (bytes_sent == -1 && errno != EAGAIN) {
        return -SEND_FAIL;
      } else {
        total_bytes_piped += bytes_sent;
        bytes_read -= bytes_sent;
        bytes_sent_this_round += bytes_sent;
        if (bytes_read == 0) {
          break;
        }
      }
    }
  }
}

ProxyScheduler::ProxyScheduler(std::vector<unsigned> threads_per_prio,
                               bool set_thread_priority,
                               CommunicationHandlerInterface* comm_interface)
    : Scheduler<std::unique_ptr<TcpPipe>>(threads_per_prio,
                                          set_thread_priority),
      _comm_interface(comm_interface),
      _destructing(false),
      _worker_exec(kWorkerThreadpoolSize) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

ProxyScheduler::~ProxyScheduler() {
  VLOG(4) << __PRETTY_FUNCTION__;
  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }

  // Releasing threads from blocking MultiQueue call.
  if (_running) {
    _multi_q_p->ReleaseQueues();
  }
}

void ProxyScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    // Convert the UniqJobPtr to SharedJobPtr to allow capture in
    // closure. Note that uniq_ptr's are are hard/impossible to capture using
    // std::bind as they do not have a copy constructor.
    SharedJobPtr<std::unique_ptr<TcpPipe>> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Proxy scheduling " << job->Describe() << " "
            << job->job_data->Describe();

    // Clear resources of input.
    std::function<void()> del = job->job_data->in->deleter;

    // Send server connection to connect.
    CallBack2 connected(std::bind(&dans::ProxyScheduler::ConnectCallbackWrapper,
                                  this, job, std::placeholders::_1,
                                  std::placeholders::_2));

    job->job_data->out = std::make_unique<HalfPipe>(_comm_interface->Connect(
        ip, job->priority == 0 ? kHighPrioPort : kLowPrioPort,
        std::move(connected)));

    // Send client connection to monitor
    CallBack2 monitored(std::bind(&dans::ProxyScheduler::MonitorCallbackWrapper,
                                  this, job, std::placeholders::_1,
                                  std::placeholders::_2));

    job->job_data->in->deleter =
        _comm_interface->Monitor(job->job_data->in->connection->Socket(),
                                 {false, false}, std::move(monitored));

    del();
  }
}

// Called when the server is connected
void ProxyScheduler::ConnectCallbackWrapper(
    SharedJobPtr<std::unique_ptr<TcpPipe>> job, int soc, ReadyFor ready_for) {
  VLOG(2) << __PRETTY_FUNCTION__ << " " << job->Describe() << " "
          << job->job_data->Describe() << " for server";
  // Send to _worker_exec for execution.
  _worker_exec.Submit({std::bind(&dans::ProxyScheduler::ConnectCallback, this,
                                 job, soc, ready_for),
                       job->job_id});
}

// Called when the server is connected
void ProxyScheduler::ConnectCallback(SharedJobPtr<std::unique_ptr<TcpPipe>> job,
                                     int soc, ReadyFor ready_for) {
  VLOG(2) << __PRETTY_FUNCTION__ << " " << job->Describe() << " "
          << job->job_data->Describe() << " for server";

  // Adding connection of the server.
  job->job_data->out->connection = std::make_unique<Connection>(soc);

  if (ready_for.err != 0) {
    PLOG(WARNING) << "Failed to create TCP connection for server "
                  << job->Describe() << " " << job->job_data->Describe();
    job->job_data->ShutdownOther(soc);
    return;
  } else if (!ready_for.out) {
    VLOG(2) << "Failed to create TCP connection for server " << job->Describe()
            << " " << job->job_data->Describe();
    job->job_data->ShutdownOther(soc);
    return;
  }

  VLOG(2) << "Proxy created TCP pipe for client-server " << job->Describe()
          << " " << job->job_data->Describe();
  // Save outdated resources deleter of client callback.
  std::function<void()> del = job->job_data->in->deleter;
  CallBack2 monitored(std::bind(&dans::ProxyScheduler::MonitorCallbackWrapper,
                                this, job, std::placeholders::_1,
                                std::placeholders::_2));
  // Send client connection to monitor
  job->job_data->in->deleter = _comm_interface->Monitor(
      job->job_data->in->connection->Socket(), {true, false}, monitored);
  // Send server connection to monitor
  job->job_data->out->deleter = _comm_interface->Monitor(
      job->job_data->out->connection->Socket(), {true, false}, monitored);
  // Clear outdated resources of client callback.
  del();
}

void ProxyScheduler::MonitorCallbackWrapper(
    SharedJobPtr<std::unique_ptr<TcpPipe>> job, int soc, ReadyFor ready_for) {
  VLOG(2) << __PRETTY_FUNCTION__ << " " << job->Describe() << " "
          << job->job_data->Describe() << " for " << job->job_data->Which(soc);
  // Send to _worker_exec for execution.
  _worker_exec.Submit({std::bind(&dans::ProxyScheduler::MonitorCallback, this,
                                 job, soc, ready_for),
                       job->job_id});
}

void ProxyScheduler::MonitorCallback(SharedJobPtr<std::unique_ptr<TcpPipe>> job,
                                     int soc, ReadyFor ready_for) {
  VLOG(2) << __PRETTY_FUNCTION__ << " " << job->Describe() << " "
          << job->job_data->Describe() << " for " << job->job_data->Which(soc);

  if (ready_for.err != 0) {
    VLOG(2) << job->Describe() << " " << job->job_data->Describe()
            << " complete";
    job->job_data->ShutdownOther(soc);
    return;
  } else if (!ready_for.in) {
    VLOG(2) << job->Describe() << " " << job->job_data->Describe() << " for "
            << job->job_data->Which(soc)
            << " was not ready for input when triggered.";
    job->job_data->ShutdownOther(soc);
    return;
  }

  VLOG(3) << job->Describe() << " " << job->job_data->Describe()
          << " Proxy found " << job->job_data->Which(soc)
          << " ready for reading.";

  int bytes_piped = job->job_data->Pipe(soc);
  if (bytes_piped < 0 && bytes_piped != -TcpPipe::TRY_LATER) {
    if (bytes_piped == -soc) {
      PLOG(WARNING) << job->Describe() << " Error reading from "
                    << job->job_data->Which(soc) << " while "
                    << job->job_data->Describe();
      job->job_data->ShutdownOther(soc);
    } else {
      PLOG(WARNING) << job->Describe() << " Error sending to "
                    << job->job_data->Which(job->job_data->OtherSocket(soc))
                    << " while " << job->job_data->Describe();
    }
  } else if (bytes_piped == 0) {
    VLOG(2) << job->Describe() << " " << job->job_data->Which(soc)
            << " closed connection on " << job->job_data->Describe();
    job->job_data->ShutdownOther(soc);
  } else {
    bytes_piped = bytes_piped == -TcpPipe::TRY_LATER ? 0 : bytes_piped;
    VLOG(3) << job->Describe() << " piped " << bytes_piped << " bytes from "
            << job->job_data->Which(soc) << " on " << job->job_data->Describe();
    // Sent some bytes
    CallBack2 monitored(std::bind(&dans::ProxyScheduler::MonitorCallbackWrapper,
                                  this, job, std::placeholders::_1,
                                  std::placeholders::_2));
    // Register event monitoring again.
    job->job_data->in->deleter =
        _comm_interface->Monitor(soc, {true, false}, std::move(monitored));
  }
}

}  // namespace dans