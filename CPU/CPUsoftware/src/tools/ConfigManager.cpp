#include "ConfigManager.h"

/** constructor
 * initialise the file paths and configuration output
 */
ConfigManager::ConfigManager () {
  this->config_file_local = CONFIG_FILE_LOCAL;
  this->config_file_usb0 = CONFIG_FILE_USB0;
  this->config_file_usb1 = CONFIG_FILE_USB1;

  this->Initialise();
}

/**
 * constructor
 * @param cfl path to the local configuration file
 * @param cf path to the configuration file to be copied over 
 */
ConfigManager::ConfigManager (const std::string & cfl, const std::string & cf0, const std::string & cf1) {
  this->config_file_local = cfl;
  this->config_file_usb0 = cf0;
  this->config_file_usb1 = cf1;

  this->Initialise();
}

void ConfigManager::Initialise(){

  this->ConfigOut = std::make_shared<Config>();
  
  /* initialise struct members to -1 */
  this->ConfigOut->cathode_voltage = -1;
  this->ConfigOut->dynode_voltage_string = "";
  this->ConfigOut->scurve_start = -1;
  this->ConfigOut->scurve_step = -1;
  this->ConfigOut->scurve_stop = -1;
  this->ConfigOut->scurve_acc = -1;
  this->ConfigOut->dac_level = -1;
  this->ConfigOut->N1 = -1;
  this->ConfigOut->N2 = -1;
  this->ConfigOut->L2_N_BG = -1;
  this->ConfigOut->L2_LOW_THRESH = -1;
  this->ConfigOut->arduino_wait_period =-1;
  this->ConfigOut->ana_sensor_num = -1;
  this->ConfigOut->average_depth = -1;
  this->ConfigOut->day_light_threshold = -1;
  this->ConfigOut->night_light_threshold = -1;
  this->ConfigOut->light_poll_time = -1;
  this->ConfigOut->light_acq_time = -1;
  this->ConfigOut->status_period = -1;
  this->ConfigOut->pwr_on_delay =-1;
  this->ConfigOut->camera_on =-1;
  
  /* initialise HV switch to be set by InputParser */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->hv_on = false;

  /* initialise instrument and acquisition modes to be set by RunInstrument */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->instrument_mode = 0;
  this->ConfigOut->acquisition_mode = 0;
  this->ConfigOut->hvps_log_len = 0;
  
}

/**
 * copy a file 
 * @param SRC path to source
 * @param DEST path to destination 
 */
bool ConfigManager::CopyFile(const char * SRC, const char * DEST) { 

  std::ifstream src(SRC, std::ios::binary);
  std::ofstream dest(DEST, std::ios::binary);

  /* check file exists */
  if (src.good() && dest.good()) {
    clog << "info: " << logstream::info << "copying " << SRC << " to " << DEST << std::endl; 
    dest << src.rdbuf();
  }
  else {
    if (!src.good()) {
      clog << "error: " << logstream::error << "cannot find file to copy" << std::endl; 
      std::cout << "ERROR: cannot find config file to copy" << std::endl;
    }
    if (!src.good()) {
      clog << "error: " << logstream::error << "cannot find destination to copy to" << std::endl; 
      std::cout << "ERROR: cannot find config file destination to copy to" << std::endl;
    }
  }
  return src && dest;
}

/**
 * parse the configuration file 
 */
void ConfigManager::Parse(std::string config_file_name){

  std::string line;  
  std::ifstream cfg_file;

  cfg_file.open(config_file_name.c_str());

  if (cfg_file.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(cfg_file, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;

      if (type == "CATHODE_VOLTAGE") {
	in >> this->ConfigOut->cathode_voltage;
      }
      else if (type == "DYNODE_VOLTAGE") {
	int dv;
	in >> dv;
	this->ConfigOut->dynode_voltage_string = CpuTools::BuildStr("", ",", dv, N_EC); 
      }
      else if (type == "SCURVE_START") {
	in >> this->ConfigOut->scurve_start;
      }
      else if (type == "SCURVE_STEP") {
	in >> this->ConfigOut->scurve_step;
      }
      else if (type == "SCURVE_STOP") {
	in >> this->ConfigOut->scurve_stop;
      }
      else if (type == "SCURVE_ACC") {
	in >> this->ConfigOut->scurve_acc;
      }
      else if (type == "DAC_LEVEL") {
	in >> this->ConfigOut->dac_level;
      } 
      else if (type == "N1") {
	in >> this->ConfigOut->N1;
      }
      else if (type == "N2") {
	in >> this->ConfigOut->N2;
      }
      else if (type == "L2_N_BG") {
	in >> this->ConfigOut->L2_N_BG;
      }
      else if (type == "L2_LOW_THRESH") {
	in >> this->ConfigOut->L2_LOW_THRESH;
      }
      else if (type == "ARDUINO_WAIT_PERIOD") {
	in >> this->ConfigOut->arduino_wait_period;
      }      
      else if (type == "ANA_SENSOR_NUM") {
	in >> this->ConfigOut->ana_sensor_num;
      }
      else if (type == "AVERAGE_DEPTH") {
	in >> this->ConfigOut->average_depth;
      }
      else if (type == "DAY_LIGHT_THRESHOLD") {
	in >> this->ConfigOut->day_light_threshold;
      }
      else if (type == "NIGHT_LIGHT_THRESHOLD") {
	in >> this->ConfigOut->night_light_threshold;
      }
      else if (type == "LIGHT_POLL_TIME") {
	in >> this->ConfigOut->light_poll_time;
      }
      else if (type == "LIGHT_ACQ_TIME") {
	in >> this->ConfigOut->light_acq_time;
      }
      else if (type == "STATUS_PERIOD") {
	in >> this->ConfigOut->status_period;
      }
      else if (type == "PWR_ON_DELAY") {
	in >> this->ConfigOut->pwr_on_delay;
      }
      else if (type == "CAMERA_ON") {
	in >> this->ConfigOut->camera_on;
      }
      
    }
    cfg_file.close();
    	
  }
  printf("\n");
     
}


/**
 * reload and parse the configuration file 
 */
void ConfigManager::Configure() {

  clog << "info: " << logstream::info << "running configuration" << std::endl;

  std::stringstream cflocal;
  std::string config_file_local_name;
  std::ifstream cfg_file_local;
  cflocal << this->config_file_local;
  config_file_local_name = cflocal.str();
 
  std::stringstream cfusb0;
  std::string config_file_usb0_name;
  std::ifstream cfg_file_usb0;
  cfusb0 << this->config_file_usb0;
  config_file_usb0_name = cfusb0.str();

  std::stringstream cfusb1;
  std::string config_file_usb1_name;
  std::ifstream cfg_file_usb1;
  cfusb1 << this->config_file_usb1;
  config_file_usb1_name = cfusb1.str();


  cfg_file_local.open(config_file_local_name.c_str());
  if (cfg_file_local.is_open()) {
    Parse(config_file_local_name);
    cfg_file_local.close();
    clog << "info: " << logstream::info << "Local configuration file parsed" << config_file_local << std::endl;
    std::cout << "Local configuration file parsed: " << this->config_file_local << std::endl;
  }
  else {  
    clog << "error: " << logstream::error << "Local configuration file " << config_file_local << " does not exist: start configuration from usb" << std::endl;
    std::cout << "ERROR: local configuration file " << config_file_local << " does not exist: start configuration from usb" << std::endl;
  }
  
  
  cfg_file_usb0.open(config_file_usb0_name.c_str());
  if (cfg_file_usb0.is_open()) {
    Parse(config_file_usb0_name);
    cfg_file_usb0.close();
    clog << "info: " << logstream::info << "Usb0 configuration file parsed: " << this->config_file_usb0 << std::endl;
    std::cout << "Usb0 configuration file parsed: " << this->config_file_usb0 << std::endl;
  }
  else { 
    clog << "error: " << logstream::error << "Usb0 configuration file " << config_file_usb0 << " does not exist: try configuration from usb1" << std::endl;
    std::cout << "ERROR: Usb0 configuration file " << config_file_usb0 << " does not exist: trying configuration from usb1" << std::endl;
    cfg_file_usb1.open(config_file_usb1_name.c_str());
    if (cfg_file_usb1.is_open()) {
      Parse(config_file_usb1_name);
      cfg_file_usb1.close();
      clog << "info: " << logstream::info << "Usb1 configuration file parsed: " << this->config_file_usb1 << std::endl;
      std::cout << "Usb1 configuration file parsed: " << this->config_file_usb1 << std::endl;
    }
    else { 
      clog << "error: " << logstream::error << "Usb1 configuration file " << config_file_usb1 << " does not exist: local configuration file parsed" << std::endl;
      std::cout << "ERROR: Usb1 configuration file " << config_file_usb1 << " does not exist: local configuration file parsed" << std::endl;    
    }
  } 

  
  return;
}

/**
 * check a configuration file has indeed been parsed, by checking output != -1
 * @param ConfigOut the output to be checked
 */
bool ConfigManager::IsParsed() {

  if (this->ConfigOut->cathode_voltage != -1 &&
      !this->ConfigOut->dynode_voltage_string.empty() &&
      this->ConfigOut->scurve_start != -1 &&
      this->ConfigOut->scurve_step != -1 &&
      this->ConfigOut->scurve_acc != -1 &&
      this->ConfigOut->dac_level != -1 &&
      this->ConfigOut->N1 != -1 &&
      this->ConfigOut->N2 != -1 &&
      this->ConfigOut->L2_N_BG != -1 &&
      this->ConfigOut->L2_LOW_THRESH != -1 &&
      this->ConfigOut->arduino_wait_period != -1 &&
      this->ConfigOut->ana_sensor_num != -1 &&
      this->ConfigOut->average_depth != -1 &&
      this->ConfigOut->day_light_threshold != -1 &&
      this->ConfigOut->night_light_threshold != -1 &&
      this->ConfigOut->light_poll_time != -1 &&
      this->ConfigOut->light_acq_time != -1 &&
      this->ConfigOut->status_period != -1 &&
      this->ConfigOut->pwr_on_delay != -1 &&
      this->ConfigOut->camera_on != -1) {
    
    return true;
  }
  else {
    return false;
  }
}
