#ifndef _DATA_REDUCTION_H
#define _DATA_REDUCTION_H

#include <thread>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

#include "log.h"
#include "OperationMode.h"
#include "AnalogManager.h"
#include "ThermManager.h"
#include "ConfigManager.h"


/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */


/**
 * DAY operational mode: data reduction 
 * class to handle data reduction and preparation of diagnostic samples 
 * to be sent to Earth and check the instrument is operating correctly
 * @TODO add main functionality
 */
class DataReduction : public OperationMode {
public:
  
  DataReduction();
  void Start();

  /**
  * output of the configuration parsing is stored here
  */
  std::shared_ptr<Config> ConfigOut;


private:

  int RunDataReduction();
  
};

#endif
/* DATA_REDUCTION_H */
