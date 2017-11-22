
#include "common/application/configreader.h"

#include <cstdlib>          // For EXIT_SUCCESS and EXIT_FAILURE
#include <iostream>
#include <fstream>

namespace {
  // Each variable name should be the same name as some member of a
  // struct member. The value should be initialized to the name being read from
  // the config file that corresponds to said member. In general, they should
  // be the same.

  // Used in struct Config
  const std::string dn_count("dn_count");
  const std::string cli_count("cli_count");
  const std::string btlnk_bw("btlnk_bw");
  const std::string file_size("file_size");               
  const std::string pLoad("pLoad");               
  const std::string scheme("scheme");               
  const std::string flow_arrival("flow_arrival");               
  const std::string schemes_count("schemes_count");
  const std::string load_count("load_count");
  const std::string sim_time("sim_time");
  const std::string min_fetch_count("min_fetch_count");
  const std::string run_count("run_count");
  const std::string file_data_set("file_data_set");
  const std::string pri_fetch_count("pri_fetch_count");
  const std::string sec_fetch_count("sec_fetch_count");
  const std::string purging("purging");
}

bool ReadConfig(Config *config, char *config_filename) {
  int value;
  std::string key;

  // Zeroing out config struct.
  *config = Config();

  // Open config file for reading.
  std::ifstream config_file(config_filename);
  if (!config_file.is_open()) {
    std::cerr << "ERROR: config file does not exist.\n";
    return false;
  }

  while((config_file) >> key >> value) {
    std::cout << key << " " << value << std::endl;

    if (key.compare(dn_count) == 0) config->dn_count = value;
    else if (key.compare(cli_count) == 0) config->cli_count = value;
    else if (key.compare(btlnk_bw) == 0) config->btlnk_bw = value;
    else if (key.compare(file_size) == 0) config->file_size = value;
    else if (key.compare(pLoad) == 0) config->pLoad = value;
    else if (key.compare(scheme) == 0) config->scheme = value;
    else if (key.compare(sim_time) == 0) config->sim_time = value;
    else if (key.compare(min_fetch_count) == 0) config->min_fetch_count = value;
    else if (key.compare(file_data_set) == 0) config->file_data_set = value;
    else if (key.compare(purging) == 0) config->purging = value;
    else if (key.compare(flow_arrival) == 0) config->flow_arrival = value;
    else if (key.compare(run_count) == 0) config->run_count = value;
    else {
      std::cerr << "ERROR: unknown config key."
        << " Key=" << key
        << " Value=" << value << std::endl;
      return false;
    }
  }

  config_file.close();
  return true;
}