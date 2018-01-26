
#include "common/application/configreader.h"

#include <fstream>
#include <iostream>

namespace {
// Each variable name should be the same name as some member of a
// struct member. The value should be initialized to the name being read from
// the config file that corresponds to said member. In general, they should
// be the same.

// Used in struct ClientConfig
const std::string btlnk_bw("btlnk_bw");
const std::string file_size("file_size");
const std::string p_load("p_load");
const std::string scheme("scheme");
const std::string flow_arrival("flow_arrival");
const std::string sim_time("sim_time");
const std::string run_count("run_count");
const std::string file_data_set("file_data_set");
const std::string server("server");
}  // namespace

namespace common_application {

bool ReadClientConfig(ClientConfig *config, char *config_filename) {
  int int_value;
  Server server_value;
  std::string key;

  // Zeroing out config struct.
  *config = ClientConfig();

  // Open config file for reading.
  std::ifstream config_file(config_filename);
  if (!config_file.is_open()) {
    std::cerr << "ERROR: config file does not exist.\n";
    return false;
  }

  while ((config_file) >> key) {
    if (key.compare(server) == 0) {
      config_file >> server_value.server_id >> server_value.ip_addr >>
          server_value.pri_port_no >> server_value.sec_port_no;
      int_value = -1;
      std::cout << key << " " << server_value.server_id << " "
                << server_value.ip_addr << " " << server_value.pri_port_no
                << " " << server_value.sec_port_no << std::endl;
    } else {
      config_file >> int_value;
      std::cout << key << " " << int_value << std::endl;
    }

    if (key.compare(btlnk_bw) == 0)
      config->btlnk_bw = int_value;
    else if (key.compare(file_size) == 0)
      config->file_size = int_value;
    else if (key.compare(p_load) == 0)
      config->p_loads.push_back(int_value);
    else if (key.compare(scheme) == 0)
      config->schemes.push_back(int_value);
    else if (key.compare(sim_time) == 0)
      config->sim_time = int_value;
    else if (key.compare(file_data_set) == 0)
      config->file_data_set = int_value;
    else if (key.compare(flow_arrival) == 0)
      config->flow_arrival = int_value;
    else if (key.compare(run_count) == 0)
      config->run_count = int_value;
    else if (key.compare(server) == 0)
      config->servers.push_back(server_value);
    else {
      std::cerr << "ERROR: unknown config key."
                << " Key=" << key << std::endl;
      return false;
    }
  }

  // require min p_load, scheme and run_count.
  config_file.close();
  return true;
}

}  // namespace common_application