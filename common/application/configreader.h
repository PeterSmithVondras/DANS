// Used for grabbing configuration files for application level testing.

#ifndef COMMON_APPLICATION_CONFIGREADER_H
#define COMMON_APPLICATION_CONFIGREADER_H

#include <iostream>
#include <vector>

namespace common_application {

struct Server
{
  int server_id;
  std::string ip_addr;
  int pri_port_no;
  int sec_port_no;
};

struct ClientConfig
{
  int btlnk_bw; // Bottleneck bandwidth Mbps
  int file_size; // Filesize in megabytes
  std::vector<int> p_loads; // Percent load
  std::vector<int> schemes; //0->Base, 1->Duplicate, 2->Priority
  int flow_arrival; //0->ClosedLoop, 1->POISSON, 2->UNIFORM, 3->Pri-CL, Sec-POISSON, 4->Pri-POISSON, Sec-CL
  int sim_time; // simulation time in seconds
  int run_count;
  int file_data_set;

  std::vector<Server> servers;
};

// Given a struct Config and a filename, puts values of config file into
// appropriate location in struct. Writes zeros to unset values. Returns true
// on success and false on failure to open file or bad key.
bool ReadClientConfig(ClientConfig *config, char *config_filename);

} // namespace common_application

#endif // COMMON_APPLICATION_CONFIGREADER_H