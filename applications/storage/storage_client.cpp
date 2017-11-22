// Storage Client V0.01
// Client side application in storage problem.

#include <cstdlib>     // For EXIT_SUCCESS and EXIT_FAILURE
#include <iostream>
#include <fstream>

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

  Config config;
  if (ReadConfig(&config, argv[1]) == false){
    USAGE(argv[0])
    return EXIT_FAILURE;
  }

  std::cout << "Info:: Successfully read configuration files\n";

  return EXIT_SUCCESS;
}