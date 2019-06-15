#include "ThermManager.h"

/**
 * constructor. 
 * initailses cpu_file_is_set to false
 */
ThermManager::ThermManager() { 

  this->cpu_file_is_set = false;
  this->inst_mode_switch = false;

}

/**
 * initialise the thermistors and write addresses to init file
 */
void ThermManager::Init() {

  const char * cmd = "digitemp -s /dev/ttyS0 -i";
  std::string output = CpuTools::CommandToStr(cmd);

}

/**
 * get the temperature by running the digitemp command and parsing the output
 */
TempAcq * ThermManager::GetTemperature() {
 
  /* define command to read temperature from all thermistors on COM port 1 */
  const char * cmd = "digitemp -s /dev/ttyS0 -a";

  /* run the digitemp command */
  std::string output = CpuTools::CommandToStr(cmd);
  
  TempAcq * temperature_result;
  size_t found = output.find("Error 10:");
  if (found != std::string::npos) {
    clog << "error: " << logstream::error << "cannot connect to temprature sensors, writing 0 to output." << std::endl;
    temperature_result = ParseDigitempOutput("output_error");
  }
  else {
    /* parse the output */
    temperature_result = ParseDigitempOutput(output);
  }
  
  return temperature_result;
}

/**
 * print the temperature for use with debugging
 */
void ThermManager::PrintTemperature() {
 
  Init();

  /* define command to read temperature from all thermistors on COM port 1 */
  const char * cmd = "digitemp -s /dev/ttyS0 -a";

  /* run the digitemp command */
  std::string output = CpuTools::CommandToStr(cmd);
  
 size_t found = output.find("Error 10:");
 if (found != std::string::npos) {
   clog << "error: " << logstream::error << "cannot connect to temprature sensors" << std::endl;
 }
 else {
   /* print the output */
   std::cout << output << std::endl;
 }
 
}

/**
 * parse the digitemp output 
 * @param input_string the string containing the output of the digitemp command
 */
TempAcq * ThermManager::ParseDigitempOutput(std::string input_string) {

  std::regex num_with_two_dp("([0-9]+\\.[0-9]{2})");
  std::smatch match;
  TempAcq * temperature_result = new TempAcq();

  /* check for null output */
  int k = 0;
  if (input_string == "output_error") {
    for (k = 0; k < N_CHANNELS_THERM; k++) {
      temperature_result->val[k] = 99;
    }    
  }
  else {
  
    /* search for numbers with 2 decimal places */ 
    std::string::const_iterator searchStart(input_string.cbegin());
    int i = 0;
    int j = 0;
    while (std::regex_search(searchStart, input_string.cend(), match, num_with_two_dp)) {
      
      /* fill the results for even values only (ignore Fahrenheit results) */
      if (i % 2 == 0) {
	temperature_result->val[j] = std::stof(match[0]);
	j++;
      }
      
      i++;
      searchStart += match.position() + match.length();
    }
  }
  
  return temperature_result;
}

/*
 * write the temperature packet to file 
 * @param temperature_results contains the parsed temperature data
 */
int ThermManager::WriteThermPkt(TempAcq * temperature_result) {

  THERM_PACKET * therm_packet = new THERM_PACKET();
  static unsigned int pkt_counter = 0;
  
  clog << "info: " << logstream::info << "writing new therm packet to " << this->RunAccess->path << std::endl;
  /* create the therm packet header */
  therm_packet->therm_packet_header.header = CpuTools::BuildCpuHeader(THERM_PACKET_TYPE, THERM_PACKET_VER);
  therm_packet->therm_packet_header.pkt_size = sizeof(*therm_packet);
  therm_packet->therm_packet_header.pkt_num = pkt_counter; 
  therm_packet->therm_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();

  if (temperature_result != NULL) {
    /* get the temperature data */
    for (int i = 0; i < N_CHANNELS_THERM; i++) {
      therm_packet->therm_data[i] = temperature_result->val[i];
    }
  }
  delete temperature_result;
  
  /* write the therm packet */
  this->RunAccess->WriteToSynchFile<THERM_PACKET *>(therm_packet, SynchronisedFile::CONSTANT);
  delete therm_packet; 
  pkt_counter++;

  return 0;
}

/* 
 * process to collect temperature every THERM_ACQ_SLEEP seconds (defined in ThermManager.h)
 */
int ThermManager::ProcessThermData() {

  std::mutex m;
  time_t start_time = time(0);
  time_t time_diff = 0;
  bool first_run = true;

  
  std::unique_lock<std::mutex> lock(this->m_mode_switch);
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(WAIT_PERIOD),
				       [this] { return this->inst_mode_switch; })) { 
  
    if ((time_diff > THERM_ACQ_SLEEP) || first_run) {
      /* collect data */
      TempAcq * temperature_result = GetTemperature();
      
      /* wait for CPU file to be set by DataAcqManager::ProcessIncomingData() */
      std::unique_lock<std::mutex> lock(m);
      this->cond_var.wait(lock, [this]{return cpu_file_is_set == true;});
      
    
      /* write to file */
      if (temperature_result != NULL) {
	WriteThermPkt(temperature_result);
      }
      
      first_run = false;
      start_time = time(0);
    }
    
    /* sleep */
    sleep(THERM_ACQ_CHECK);
    time_diff = time(0) - start_time;

  }
 
  return 0;
}


/**
 * reset the mode switching after an instrument mode change
 * used by OperationMode::Reset() 
 */
int ThermManager::Reset() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = false;
  } /* release mutex */

  return 0;
}

/**
 * notify the object of an instrument mode switch 
 * used by OperationMode::Notify
 */
int ThermManager::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = true;
  } /* release mutex */
  
  return 0;
}
