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

  this->ConfigOut = std::make_shared<Config>();
  
  /* launch thread */
  std::thread data_reduction (&DataReduction::RunDataReduction, this);

  /* launch the analog acquisition */
  std::thread analog(&AnalogManager::ProcessAnalogData, this->Analog, ConfigOut); 
  analog.join();

  
  /* wait for thread to exit, when instrument mode switches */
  data_reduction.join();
  
  return;
}

/**
 * data reduction procedure 
 * compresses data into .zip files
 */
int DataReduction::RunDataReduction() {

  std::string output;
  const char * data_compression_cmd;
  std::stringstream conv;
  
  std::unique_lock<std::mutex> lock(this->_m_switch); 

  /* enter loop while instrument mode switching not requested */
  while(!this->_cv_switch.wait_for(lock,
				   std::chrono::milliseconds(WAIT_PERIOD),
				   [this] { return this->_switch; } )) {   

    /* build command */
    conv << "sh /home/software/data_reduction/ZipDemon.sh" << std::endl;

    /* convert stringstream to char * */
    data_compression_cmd = conv.str().c_str();
  
    /* run script to compress data */
    /* should be something short (~10s of seconds max) which exits by itself so we can check for a mode switch */
    /* should also add a timeout to be safe */
    /* can read the output string to debug/do stuff */
    output = CpuTools::CommandToStr(data_compression_cmd);
    
  }
  
  return 0;
}

