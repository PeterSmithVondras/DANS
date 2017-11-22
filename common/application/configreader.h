// Used for grabbing configuration files for application level testing.

#ifndef COMMON_APPLICATION_CONFIGREADER_H
#define COMMON_APPLICATION_CONFIGREADER_H

struct Config
{
  int dn_count;
  int cli_count;
  int btlnk_bw;
  int file_size;
  int pLoad;
  int scheme;
  int *load;
  int *schemes;     //0->Base, 1->Duplicate, 2->Priority
  int flow_arrival; //0->ClosedLoop, 1->POISSON, 2->UNIFORM, 3->Pri-CL, Sec-POISSON, 4->Pri-POISSON, Sec-CL
        
  int schemes_count;
  int load_count;

  int sim_time;
  int min_fetch_count;

  int run_count;
  int file_data_set;

  int pri_fetch_count;
  int sec_fetch_count;

  int purging;
};

// Given a struct Config and a filename, puts values of config file into
// appropriate location in struct. Writes zeros to unset values. Returns true
// on success and false on failure to open file or bad key.
bool ReadConfig(Config *config, char *config_filename);

#endif // COMMON_APPLICATION_CONFIGREADER_H