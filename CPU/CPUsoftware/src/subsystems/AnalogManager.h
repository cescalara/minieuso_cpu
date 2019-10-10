#ifndef _ANALOG_MANAGER_H
#define _ANALOG_MANAGER_H

#include <mutex>
#include <memory>
#include <thread>
#include <cstring>
#include <condition_variable>
#include <termios.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "log.h"

#include "SynchronisedFile.h"
#include "ConfigManager.h"
#include "CpuTools.h"
#include "minieuso_data_format.h"

/* for use with arduino readout functions */
#define DUINO "/dev/ttyACM0"
#define BAUDRATE B9600
#define BUF_SIZE 14
#define FIFO_DEPTH 1
#define CHANNELS (N_CHANNELS_PHOTODIODE+N_CHANNELS_SIPM+N_CHANNELS_THERM)

// coming from the .h of arduino 
#define X_HEADER_SIZE 4 // AA55AA55
#define X_SIPM_BUF_SIZE 64 // 64 channels, two byte
#define X_OTHER_SENSORS 4 // 4 channels, two byte
#define X_TOTAL_BUF_SIZE (X_SIPM_BUF_SIZE+X_OTHER_SENSORS)
// packet number at begin and crc at end
#define X_TOTAL_BUF_SIZE_HEADER (X_HEADER_SIZE+X_SIPM_BUF_SIZE+X_OTHER_SENSORS+4+90+56+90+56) 
//#define X_TOTAL_BUF_SIZE_HEADER (X_HEADER_SIZE+X_SIPM_BUF_SIZE+X_OTHER_SENSORS+4) 

#define X_DELAY 100 // ms
#define READ_ARDUINO_TIMEOUT  1000 // it should be in ms now is in attempts to read the buffer

/**
 * acquisition structure to store analog readout 
 */
typedef struct {
  unsigned int val [FIFO_DEPTH][CHANNELS];
} AnalogAcq;

/**
 * acquisition structure for temperature readout 
 */
typedef struct {
  float val [N_CHANNELS_THERM];
} TemperatureAcq;

/**
 * struct to store light levels for polling 
 */
typedef struct {
  float photodiode_data[N_CHANNELS_PHOTODIODE];  
  float sipm_data[N_CHANNELS_SIPM]; 
  float sipm_single; 
} LightLevel;

/**
 * class to handle the analog data acquisition (photodiodes, SiPMs, thermistors) 
 * uses a simple Arduino to replace the AnalogManager module due to 
 * issues with power comsumption.
 */
class AnalogManager {
public:
  /*
   * SynchronisedFile access
   */
  Access * RunAccess;
  /*
   * to wait for CPU file to be created by DataAcquisition::CreateCpuRun
   */
  std::condition_variable cond_var;
  /*
  * to notify that the CPU file is set by DataAcquisition::CreateCpuRun
  */
  bool cpu_file_is_set;


  AnalogManager();
  std::shared_ptr<LightLevel> ReadLightLevel();
 
  /**
   * enum to specify the current light level status of the instrument
   */
  enum LightLevelStatus : uint8_t {
	  LIGHT_BELOW_NIGHT_THR = 0,
	  LIGHT_ABOVE_DAY_THR = 1,
	  LIGHT_UNDEF = 2,
  };

  LightLevelStatus current_lightlevel_status;

  LightLevelStatus CompareLightLevel(std::shared_ptr<Config> ConfigOut);
  int ProcessAnalogData(std::shared_ptr<Config> ConfigOut);  
  int GetLightLevel(std::shared_ptr<Config> ConfigOut);
  int GetTemperature();
  int AnalogDataCollect();
  int WriteThermPkt(); 
  
  /* handle instrument mode switching */
  int Notify();
  int Reset();
  
private:

  /*
   * for thread-safe access to the light_level 
   */
  std::mutex m_light_level;
  /*
   * light level stored here and accessed only with mutex protection
   */
  std::shared_ptr<LightLevel> light_level;
  /*
   * for thread-safe access to the analog_acq
   */
  std::mutex m_temperature_acq;
  /*
   * temperature acq stored here and accessed only with mutex protection 
   */
  std::shared_ptr<TemperatureAcq> temperature_acq;
  /*
   * analog acquisition stored here
   */
  std::shared_ptr<AnalogAcq> analog_acq;

  /*
   * to notify the object of a mode switch
   */
  bool inst_mode_switch;
  /*
   * to handle mode switching in a thread-safe way
   */
  std::mutex m_mode_switch;
  /*
   * to wait for a mode switch
   */
  std::condition_variable cv_mode_switch;

  
  int SetInterfaceAttribs(int fd, int speed);
  int SerialReadOut(int fd);
  float ConvertToTemp(char data[9]);
};

#endif
/* _ANALOG_MANAGER_H */
