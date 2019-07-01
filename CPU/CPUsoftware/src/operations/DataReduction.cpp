#include "DataReduction.h"

/** 
 * constructor 
 */
DataReduction::DataReduction() {
  
}

/**
 * launch thread to do data reduction 
 */
void DataReduction::Start() {

  clog << "info: " << logstream::info << "starting data reduction" << std::endl;
  std::cout << "starting data reduction" << std::endl;

  /* launch thread */
  std::thread data_reduction (&DataReduction::RunDataReduction, this);
  
  /* wait for thread to exit, when instrument mode switches */
  data_reduction.join();
  
  return;
}

/**
 * data reduction procedure 
 * compresses data into .zip files
 */
int DataReduction::RunDataReduction() {

  /* enter loop while instrument mode switching not requested */
  std::unique_lock<std::mutex> lock(this->_m_switch); 
  while(!this->_cv_switch.wait_for(lock,
				   std::chrono::milliseconds(WAIT_PERIOD),
				   [this] { return this->_switch; } )) {   

    std::cout << "running data reduction loop..." << std::endl;
    sleep(1);
    
  }
  
  return 0;
}

