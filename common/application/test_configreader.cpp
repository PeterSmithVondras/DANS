// Tests configreader.h V0.01
// Client side application in storage problem.

#include <cstdlib>     // EXIT_SUCCESS and EXIT_FAILURE
#include <iostream>

#include <syslog.h>    // openlog and options

#include "common/application/configreader.h"

namespace {
  #define USAGE(x) std::cerr << "usage: $ " << x << " path/to/config/file\n";
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "ERROR: config file required.\n";
    USAGE(argv[0])
    return EXIT_FAILURE;
  }

  common_application::ClientConfig config;
  if (common_application::ReadClientConfig(&config, argv[1]) == false){
    USAGE(argv[0])
    return EXIT_FAILURE;
  }

  openlog("clog", LOG_PID|LOG_CONS, LOG_LOCAL0);

  std::cout << "Info:: Successfully read configuration files\n";

  return EXIT_SUCCESS;
}