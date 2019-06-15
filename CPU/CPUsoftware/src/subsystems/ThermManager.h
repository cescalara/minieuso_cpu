#ifndef _THERM_MANAGER_H
#define _THERM_MANAGER_H

#include <regex>
#include <mutex>
#include <condition_variable>

#include "log.h"
#include "CpuTools.h"
#include "SynchronisedFile.h"


/* number of seconds between temperature acquisitions */
#define THERM_ACQ_SLEEP 60

/* number of seconds between checking for switching */
#define THERM_ACQ_CHECK 2

/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */

/**
 * acquisition structure for temperature readout 
 */
typedef struct {
  float val [N_CHANNELS_THERM];
} TemperatureAcq;


/**
 * controls the thermistor acquisition 
 */
class ThermManager {
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

  ThermManager();
  void Init();
  int ProcessThermData();
  TemperatureAcq * GetTemperature();
  int WriteThermPkt(TemperatureAcq * temperature_results);
  void PrintTemperature();

  /* handle instrument mode switching */
  int Notify();
  int Reset();

private:
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

  TemperatureAcq * ParseDigitempOutput(std::string input_string);
  
};

#endif /* _THERM_MANAGER_H */
