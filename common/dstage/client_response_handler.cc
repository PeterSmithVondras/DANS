#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>

#include "common/dstage/client_response_handler.h"
#include "glog/logging.h"

namespace {
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
const int kMBytesToBytes = 1024 * 1024;
}  // namespace

namespace dans {

ResponseScheduler::ResponseScheduler(
    std::vector<unsigned> threads_per_prio, bool set_thread_priority,
    CommunicationHandlerInterface* comm_interface,
    BaseDStage<ResponseData>* response_handler)
    : Scheduler<ResponseData>(threads_per_prio, set_thread_priority),
      _comm_interface(comm_interface),
      _response_handler(response_handler),
      _destructing(false),
      _origin_dstage(nullptr) {
  VLOG(4) << __PRETTY_FUNCTION__;
}

ResponseScheduler::~ResponseScheduler() {
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

void ResponseScheduler::RegisterOriginDStage(
    BaseDStage<ConnectData>* origin_dstage) {
  _origin_dstage = origin_dstage;
}

void ResponseScheduler::StartScheduling(Priority prio) {
  VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio;
  Protocol response_protocol;
  int bytes_read;
  bool closed;

  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    SharedJobPtr<ResponseData> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
    VLOG(1) << "Response Handler Scheduler got job_id=" << job->job_id
            << ", socket=" << job->job_data.connection->Socket();

    // Check if job has been purged
    if (job->job_data.purge_state->IsPurged()) {
      VLOG(2) << "Purged job_id=" << job->job_id
              << ", Priority=" << job->priority;
      continue;
    }

    if (job->job_data.object == nullptr) {
      bytes_read = read(job->job_data.connection->Socket(), &response_protocol,
                        sizeof(Protocol));
      CHECK_EQ(bytes_read, sizeof(Protocol));
      job->job_data.object =
          std::make_unique<std::vector<char>>(response_protocol.size_bytes);
      job->job_data.index = response_protocol.start_idx;
      CHECK_NE(response_protocol.size_bytes, 0)
          << "File must be of non-zero index.";
      if (response_protocol.type == REQUEST_ACCEPT) {
        VLOG(1) << "Server sent Accept for file="
                << response_protocol.object_id;
      } else {
        VLOG(1) << "Server sent Reject for file="
                << response_protocol.object_id;
      }
    }

    closed = false;
    while (true) {
      bytes_read = read(job->job_data.connection->Socket(),
                        job->job_data.object->data() + job->job_data.index,
                        job->job_data.object->size() - job->job_data.index);
      if (bytes_read == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          PLOG(WARNING) << "Error reading socket. job_id=" << job->job_id
                        << ", file=" << job->job_data.object_id;
          closed = true;
        }
        break;
      } else if (bytes_read == 0) {
        VLOG(3) << "Server closed socket for job_id=" << job->job_id;
        closed = true;
        break;
      }
      job->job_data.index += bytes_read;
      VLOG(3) << "Read " << job->job_data.index << " of "
              << job->job_data.object->size()
              << " bytes for file=" << job->job_data.object_id
              << ", priority=" << job->priority;
    }

    if (job->job_data.index == job->job_data.object->size()) {
      if (job->job_data.purge_state->SetPurged()) {
        VLOG(2) << "Completed job_id=" << job->job_id
                << ", priority=" << job->priority;
        (*job->job_data.done)(job->priority, job->job_data.object_id,
                              std::move(job->job_data.object));
        CHECK_NOTNULL(_origin_dstage);
        _origin_dstage->Purge(job->job_id);
      }
      continue;
    }

    if (closed) continue;

    VLOG(2) << "Monitoring for read on job_id=" << job->job_id
            << ", priority=" << job->priority
            << ", file=" << job->job_data.object_id
            << ", size=" << job->job_data.object->size()
            << ", index=" << job->job_data.index;

    CallBack2 response(std::bind(&dans::ResponseScheduler::ResponseCallback,
                                 this, job, std::placeholders::_1,
                                 std::placeholders::_2));
    _comm_interface->Monitor(job->job_data.connection->Socket(),
                             ReadyFor{/*in=*/true, /*out=*/false}, response);
  }
}

void ResponseScheduler::ResponseCallback(SharedJobPtr<ResponseData> old_job,
                                         int soc, ReadyFor ready_for) {
  VLOG(4) << __PRETTY_FUNCTION__
          << " soc=" << old_job->job_data.connection->Socket();
  CHECK(ready_for.in) << "Failed to receive response for socket="
                      << old_job->job_data.connection->Socket();

  // Pass on job if it is not complete.
  if (!old_job->job_data.purge_state->IsPurged() && soc >= 0) {
    ResponseData response_data = {std::move(old_job->job_data.connection),
                                  old_job->job_data.object_id,
                                  old_job->job_data.index,
                                  std::move(old_job->job_data.object),
                                  old_job->job_data.done,
                                  old_job->job_data.purge_state};

    auto response_job = std::make_unique<Job<ResponseData>>(
        std::move(response_data), old_job->job_id, old_job->priority,
        old_job->duplication);
    _response_handler->Dispatch(std::move(response_job),
                                /*requested_dulpication=*/0);
  } else if (soc < 0) {
    errno = -soc;
    PLOG(WARNING) << "Dropped socket for job_id=" << old_job->job_id;
  } else {
    VLOG(2) << "Purged job_id=" << old_job->job_id
            << ", Priority=" << old_job->priority;
  }
}

}  // namespace dans