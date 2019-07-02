#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <fstream>
#include <memory>

#include "log.h"
#include "CpuTools.h"

#ifndef __APPLE__

#if ARDUINO_DEBUG==1
#define CONFIG_DIR "/home/minieuso_cpu/CPU/CPUsoftware/config"
#define CONFIG_FILE_USB0 "/home/minieuso_cpu/CPU/CPUsoftware/config/dummy_usb0.conf"
#define CONFIG_FILE_USB1 "/home/minieuso_cpu/CPU/CPUsoftware/config/dummy_usb1.conf"
#define CONFIG_FILE_LOCAL "/home/minieuso_cpu/CPU/CPUsoftware/config/dummy_local.conf"
#else
#define CONFIG_DIR "/home/software/CPU/CPUsoftware/config"
#define CONFIG_FILE_USB0 "media/usb0/dummy_usb.conf"
#define CONFIG_FILE_USB1 "media/usb1/dummy_usb.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/dummy_local.conf"
#endif /* ARDUINO_DEBUG==1 */ 

#else

#define CONFIG_DIR "config"
#define CONFIG_FILE_USB0 "dummy_usb.conf"
#define CONFIG_FILE_USB1 "dummy_usb.conf"
#define CONFIG_FILE_LOCAL "dummy_local.conf"

#endif /* __APPLE__ */

//#define DIR_USB0 "/home/minieuso_cpu/CPU/CPUsoftware/config"
//#define DIR_USB1 "/home/minieuso_cpu/CPU/CPUsoftware/config"

/**
 * struct for output of the configuration file 
 */
struct Config {

  /* set in configuration file */
  int cathode_voltage;
  std::string dynode_voltage_string;
  int scurve_start;
  int scurve_step;
  int scurve_stop;
  int scurve_acc;
  int dac_level;
  int N1;
  int N2;
  int L2_N_BG;
  int L2_LOW_THRESH;
  int arduino_wait_period;
  int ana_sensor_num;
  int average_depth;
  int day_light_threshold;
  int night_light_threshold;
  int light_poll_time;
  int light_acq_time;
  int status_period;
  int pwr_on_delay;
  int camera_on;

  /* set by RunInstrument and InputParser at runtime */
  bool hv_on;
  uint8_t instrument_mode;
  uint8_t acquisition_mode;
  uint32_t hvps_log_len;
  
};

/**
 * class for configuring the instrument based on the configuration file provided 
 */
class ConfigManager {  
public:
  /**
   * path to the local configuration file
   */
  std::string config_file_local;
   /**
   * path to configuration file on USB0
   */
  std::string config_file_usb0;
   /**
   * path to configuration file on USB1
   */
  std::string config_file_usb1;
  /* /\** */
  /*  * path to configuration file to be copied */
  /*  *\/ */
  /* std::string config_file; */
  /**
   * output of the configuration parsing is stored here
   */
  std::shared_ptr<Config> ConfigOut;

  ConfigManager();
  ConfigManager(std::string, std::string, std::string);
  void Configure();
  bool IsParsed();

private:
  bool CopyFile(const char * SRC, const char * DEST);
  void Parse(std::string);

};

#endif
/* _CONFIGURATION_H */

