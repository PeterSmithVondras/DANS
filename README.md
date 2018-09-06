# Duplicate Aware Networked Stack (DANS)

A c++ library which enables the integration of _Duplicate Awareness_ into any resource that might become a bottleneck. A simple file client has been implemented which can be used to benchmark the capabilities of DANS. This Client duplicates application requests and enforces Priority and Purging through the completion cycle.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

* C++14
* Currently **DANS** requires the linux OS. It has been tested on **Ubuntu 17.10** Codename: artful. The main dependency on linux is epoll which is implemented in the [LinuxCommunicationHandler](./common/dstage/linux_communication_handler.h). To remove this dependency, create a new implementation which inherits from [CommunicationHandlerInterface](./common/dstage/communication_handler_interface.h).
* [gflags](https://github.com/gflags/gflags) for command line flag integration. NOTE that there will likely be less integration issues if you install _gflags_ **BEFORE** _glog_. For Ubuntu Use `sudo apt-get install libgflags-dev`.
* [glog](https://github.com/google/glog) for application level logging.
  * NOTE: Install _gflags_ first.
  * For ubuntu install: `sudo apt-get install libgoogle-glog-dev`
  * [Intro to glog](http://rpg.ifi.uzh.ch/docs/glog.html)
* [gtest](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md) used for all testing and even to run simple client. For Ubuntu use `sudo apt-get install googletest`.
* [RANS server](https://github.cs.tufts.edu/hmmohsin/RANS.git) with **DANS-Purge** branch installed and at least two files created for retrieval. See the RANS server repo for setup and function. Ensure that the server is installed by running `make` in the server directory.

### Installing

1. Install all Prerequisites.
2. Clone this repository.
```
git clone https://github.cs.tufts.edu/peter/DANS.git
```
3. Edit the following to capture the installed headers and source files.
  * [WORKSPACE](WORKSPACE)
  * [gflags.BUILD](gflags.BUILD)
  * [glog.BUILD](glog.BUILD)
  * [gtest.BUILD](gtest.BUILD) - NOTE that gtest is compiled with every binary and therefore cannot just link a shared library.
4. Build all targets. NOTE that the first time might take a while.
```
$ cd DANS
$ bazel build ...
```

### Going Further

Beyond installing DANS and running a simple client, see [documentation](doc) for [Architecture](./doc/architecture.md) and also [Simple Client](./doc/simple_client.md) for a description about how a simple DANS client works. Finally, see an [extensible proxy server](./applications/throttling_proxy/throttling_proxy_example.cc) which provides DANS capabilities to software that cannot be modified.

## Running the tests
Every folder is a *package.* Every *package* has a *BUILD* file which lists all targets. Any target of type *cc_test* is a test and may be run either in batch or alone. Below is a test target example:

```
cc_test(
  name = "job_test",
  size = "small",
  srcs = [ "job_test.cc" ],
  deps = [
    ":dstage_base",
    "@glog//:main",
    "@gflags//:main",
    "@gtest//:main",
  ],
)
```

Tests can be run without special flags by running `bazel test path/to/package:target`. So to run the *job_test* target in the _common/dstage_ package you might run:
```
bazel test common/dstage:job_test
``` 
Logs are saved to "/tmp/<program name>.<hostname>.<user name>.log.<severity level>.<date>.<time>.<pid>" by default, but this can be overridden at the command line.

To run a test with custom command line arguments first build the target with 

```
bazel build path/to/package:target
./bazel-bin/path/to/package/target --your_foo_flag=true
```

For quick testing it is convenient to use these flags at a minimum.
* --logtostderr=true      Route logs to stderr.
* --colorlogtostderr=true Show logs with severity schemed coloring.
* --v=0                   Log verbosity level where 0 is no verbost logs and 4 is essentially trace. This can also be controlled for every module when needed.

### Break down into end to end tests

**Package:** common/thread
Test: thread_utility_test - Tests the ability to change the priority of a thread.

```
bazel test common/thread:thread_utility_test
```

**Package:** common/dstage

Test: linux_communication_handler_test.cc - Completes a full GET request from http://google.com using an event driven reactor design leveraging linux's epoll. Note that you must have Internet connectivity to PASS this test.

```
bazel test common/dstage:linux_communication_handler_test
```

Test: job_test.cc - Simply tests the creation semantics of creating Jobs used for the dstage library.

```
bazel test common/dstage:job_test
```

Test: dispatcher_test.cc - Tests the use of the dispatcher class by creating several jobs and sending them to a dispatcher which duplicates them and passes them on.

```
bazel test common/dstage:dispatcher_test
```

Test: multiqueue_test.cc - Tests the functionality of a MultiQueue by adding jobs of several priorities, Purging a job that would be in the middle of several queues and then dequeuing the remaining jobs and ensuring that they are in order.

```
bazel test common/dstage:multiqueue_test
```

Test: scheduler_test.cc - Test the ability to create a generic scheduler with multiple threads.

```
bazel test common/dstage:scheduler_test
```

Test: dstage_test.cc - Test a single dstage which consists of a Dispatcher, MultiQueue and a Scheduler by submitting several empty jobs.

```
bazel test common/dstage:dstage_test
```

Test: file_client_dstage_chain_test.cc - Creates an entire dstage chain which constitutes a Connection, Request and Response Dstage. Then a job is created which represents a file request. This is sent to the the Connection Dstage along with a callback which will be called when the file has been retrieved. This test assumes and requires for the RANS Server to be running. If it is not running, the test will fail. Although file_client_dstage_chain_test only retrieves one file, you may increase the number of files retrieved by changing the kGetRequestsTotal. Additionally, kThreadsPerPrio can be changed which dictates the number of threads that each scheduler will use for each priority. So a single Dstage which uses two priority levels which has kThreadsPerPrio = 1 would use 2 total threads while if kThreadsPerPrio = 2, it would use 4 total threads.

```
bazel test common/dstage:file_client_dstage_chain_test
``` 

There are several flags which are helpful for this test however:
* --set_thread_priority=true Change the priority of low priority threads to be less than that of high priority threads. If this flag is used, `sudo` must also be used.
* --save_files=true will save every retrieved file as "Tx" where "x" is the file number. This will dramatically slow the test and could lead to memory issues if kGetRequestsTotal is set too high.
* --server_ip sets the ip of the server and is needed if the ip is not 192.168.137.127.

```
bazel build common/dstage:file_client_dstage_chain_test
sudo ./bazel-bin/common/dstage/file_client_dstage_chain_test --colorlogtostderr=true --logtostderr=true --v=0 --set_thread_priority=true --save_files=false --server_ip=192.168.137.127
``` 

### And coding style tests

Formats all *.cc, *.h and *.hh files to Google's C++ coding style.

```
make format
```

## Deployment

Large scale deployment, including use in emulab is not covered here at this time.

## Built With

* [Bazel](https://docs.bazel.build/versions/master/install.html) - Dependency Management

## Contributing

Please read [CONTRIBUTING](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.cs.tufts.edu/peter/DANS/tags). 

## Authors

* **Peter Vondras** - *Initial work* - [DANS](https://github.cs.tufts.edu/peter/DANS)

See also the list of [contributors](https://github.cs.tufts.edu/peter/DANS/graphs/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details

## Acknowledgments

* Special thanks to Fahad Dogar and Haffiz Mohsin Bashir.
* ReadMe template was taken from here: https://gist.github.com/PurpleBooth/109311bb0361f32d87a2
