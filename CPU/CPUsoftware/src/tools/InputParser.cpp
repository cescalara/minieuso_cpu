#include "InputParser.h"

/** 
 * constructor.
 * @param argc command line inputs
 * @param argv command line inputs
 * initalise the CmdLine struct
 */
InputParser::InputParser(int &argc, char **argv) {
  
  /* initialise the struct to handle input */
  this->CmdLine->help = false;
  this->CmdLine->ver = false;
  this->CmdLine->hvps_on = false;
  this->CmdLine->hvps_switch = false;
  this->CmdLine->debug_mode = false;
  this->CmdLine->log_on = false;
  this->CmdLine->trig_on = false;
  this->CmdLine->cam_on = false;
  this->CmdLine->cam_verbose = false;
  this->CmdLine->therm_on = false;
  this->CmdLine->lvps_on = false;
  this->CmdLine->sc_on = false;
  this->CmdLine->single_run = false;
  this->CmdLine->test_zynq_on = false;
  this->CmdLine->keep_zynq_pkt = false;
  this->CmdLine->check_status = false;
  this->CmdLine->zynq_reboot = false;
  this->CmdLine->hide_pixel = false;
  
  this->CmdLine->hvps_dv_string = "";
  this->CmdLine->asic_dac = -1;
  this->CmdLine->lvps_status = LvpsManager::UNDEF;
  this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
  this->CmdLine->hvps_status = ZynqManager::UNDEF;
  this->CmdLine->hvps_ec_string = "";
  this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
  this->CmdLine->zynq_test_mode = ZynqManager::T_NONE;

  this->CmdLine->zynq_mode_string = "";
  this->CmdLine->command_line_string = "";

  this->CmdLine->acq_len = 0;

  this->CmdLine->sc_start = -1;
  this->CmdLine->sc_step = -1;
  this->CmdLine->sc_stop = -1;
  this->CmdLine->sc_acc = -1;

  /* allowed command line options */
  this->allowed_tokens = {"-db", "-log", "-comment", "-ver", "-lvps", "-hvswitch", "-help",
			  "-dv", "-dvr", "-asicdac", "-check_status", "-cam", "-v", "-therm",
			  "-hv", "-scurve", "-start", "-stop", "-step", "-acc", "-short",
			  "-test_zynq", "-keep_zynq_pkt", "-zynq", "-subsystem", "-zynq_reboot", "-hide_pixel"};

  /* get command line input */
  std::string space = " ";
  this->CmdLine->command_line_string = "mecontrol ";
  
  for (int i = 1; i < argc; i++) {
    this->tokens.push_back(std::string(argv[i]));
    if (std::string(argv[i-1]) == "-comment") {
      this->CmdLine->command_line_string += ("\"...\"" + space);
    }
    else {
      this->CmdLine->command_line_string += (std::string(argv[i]) + space);
    }
  }

  /* initialise comment field */
  this->CmdLine->comment = "none";
  this->CmdLine->comment_fn = "";

}


/**
 * parse the command line options
 * and store the result in CmdLine 
 */
CmdLineInputs * InputParser::ParseCmdLineInputs() {

  /* check for options not being recognised */
  int check = CheckInputs();
  if (check != 0) {
    return NULL;
  } 

  /* check for help option */
  if(cmdOptionExists("-help")){
    this->CmdLine->help = true;
    PrintHelpMsg();
  }

  /* check for version info option */
  if(cmdOptionExists("-ver")){
    this->CmdLine->ver = true;
    PrintVersionInfo();
  }

  /* check for version info option */
  if(cmdOptionExists("-zynq_reboot")){
    this->CmdLine->zynq_reboot = true;
    PrintVersionInfo();
  }

  /* check for hide_pixel option */
  if(cmdOptionExists("-hide_pixel")){
    this->CmdLine->hide_pixel = true;
  }  
  
  /* check what comand line options exist */
  if(cmdOptionExists("-hv")){
    this->CmdLine->hvps_on = true;
    this->CmdLine->hvps_status = ZynqManager::ON;

 
    const std::string & hv_ec_str = getCmdOption("-hv");
    if (!hv_ec_str.empty()) {
      if (hv_ec_str == "all") {
	this->CmdLine->hvps_ec_string = "1,1,1,1,1,1,1,1,1"; 
      }
      else {
	this->CmdLine->hvps_ec_string = hv_ec_str;
      }
    }
    else {
      std::cout << "Error: for -hv option HV EC info (e.g. 1,1,1,1,1,1,1,1,1 or all) must be provided" << std::endl;
      return NULL;
    }
    
  }
  if(cmdOptionExists("-hvswitch")){
    this->CmdLine->hvps_switch = true;

    /* HVPS on/off */
    const std::string & hv_status_str = getCmdOption("-hvswitch");
    if (!hv_status_str.empty()) {
      if (hv_status_str == "on") {
	this->CmdLine->hvps_status = ZynqManager::ON;
      }
      else if (hv_status_str == "off") {
	this->CmdLine->hvps_status = ZynqManager::OFF;   
      }
      else {
	std::cout << "Error: for -hvswitch option status is not recognised -  can be on or off" << std::endl;
	return NULL;
      }
    }
    else {
	std::cout << "Error: for -hvswitch option status (on/off) must be provided" << std::endl;
	return NULL;
    }

  }
  if(cmdOptionExists("-short")){
    this->CmdLine->single_run = true;

    /* get the desired acquisition length (in no. of CPU_PACKETs) */
    const std::string & acq_len_str = getCmdOption("-short");
    if (!acq_len_str.empty()) {
      this->CmdLine->acq_len = std::stoi(acq_len_str);
      if (this->CmdLine->acq_len >= RUN_SIZE) {
	std::cout << "Error: for -short option length (int < RUN_SIZE) must be provided" << std::endl;
	return NULL;      
      }
    }
    else {
      std::cout << "Error: for -short option length (int < RUN_SIZE) must be provided" << std::endl;
      return NULL;      
    }
    
  }
  if(cmdOptionExists("-db")){
    this->CmdLine->debug_mode = true;
  }
  if(cmdOptionExists("-log")){
    this->CmdLine->log_on = true;
  }
  if(cmdOptionExists("-trig")){
    this->CmdLine->trig_on = true;
  }
  if(cmdOptionExists("-cam")){
    this->CmdLine->cam_on = true;
  }
  if(cmdOptionExists("-v")){
    this->CmdLine->cam_verbose = true;
  }
  if(cmdOptionExists("-therm")){
    this->CmdLine->therm_on = true;
  }
  if(cmdOptionExists("-lvps")){
    this->CmdLine->lvps_on = true;  

    /* LVPS on/off */
    const std::string & lvps_status_str = getCmdOption("-lvps");
    if (!lvps_status_str.empty()) {
      if (lvps_status_str == "on") {
	this->CmdLine->lvps_status = LvpsManager::ON;
      }
      else if (lvps_status_str == "off") {
	this->CmdLine->lvps_status = LvpsManager::OFF;   
      }
      else {
	std::cout << "Error: for -lvps option status is not recognised - can be on or off" << std::endl;
	return NULL;      
      }
    }
    else {
      std::cout << "Error: for -lvps option status (on/off) must be provided" << std::endl;
      return NULL;      
    }
    
    /* LVPS subsystem */    
    const std::string & subsystem_str = getCmdOption("-subsystem");
    if (!subsystem_str.empty()) {
      if (subsystem_str == "zynq") {
	this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
      }
      else if (subsystem_str == "cam") {
	this->CmdLine->lvps_subsystem = LvpsManager::CAMERAS;
      }
      else if (subsystem_str == "hk") {
	this->CmdLine->lvps_subsystem = LvpsManager::HK;
      }
      else {
	std::cout << "Error: for -subsystem option a subsystem (zynq, cam or hk) must be provided" << std::endl;
	return NULL;
      }
    }
    else {
      std::cout << "Error: for -subsystem option a subsystem must be provided - can be zynq, cam or hk" << std::endl;
      return NULL;
    }
    
  }
  if(cmdOptionExists("-scurve")){
    this->CmdLine->sc_on = true;

    const std::string & start_str = getCmdOption("-start");
    if (!start_str.empty()) {
      this->CmdLine->sc_start = std::stoi(start_str); 
    }
    const std::string & step_str = getCmdOption("-step");
    if (!step_str.empty()) {
      this->CmdLine->sc_step = std::stoi(step_str); 
    }
    const std::string & stop_str = getCmdOption("-stop");
    if (!stop_str.empty()) {
      this->CmdLine->sc_stop = std::stoi(stop_str); 
    }
    const std::string & acc_str = getCmdOption("-acc");
    if (!acc_str.empty()) {
      this->CmdLine->sc_acc = std::stoi(acc_str); 
    }
    
  }
  if(cmdOptionExists("-zynq")){

    /* zynq instrument mode */
    const std::string &mode = getCmdOption("-zynq");
    if (!mode.empty()){

      /* basic modes */
      if (mode == "none") {
	this->CmdLine->zynq_mode = ZynqManager::NONE;
      }
      else if (mode == "periodic") {
	this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
      }
      else if (mode == "self") {
	this->CmdLine->zynq_mode = ZynqManager::SELF;
      }
      else if (mode == "immediate") {
	this->CmdLine->zynq_mode = ZynqManager::IMMEDIATE;
      }
      else if (mode == "external") {
	this->CmdLine->zynq_mode = ZynqManager::EXTERNAL;
      }
      else if (mode == "trigger") {
	this->CmdLine->zynq_mode = ZynqManager::TRIGGER;
      }
      else if (mode == "ta_trigger") {
	this->CmdLine->zynq_mode = ZynqManager::TA_TRIGGER;
      }
      else {
	
	/* compound modes */
	uint8_t mode_to_set = 0;
	size_t found = mode.find(",");

	if (found != std::string::npos) {
	  found = mode.find("periodic");
	  if (found != std::string::npos) {
	    mode_to_set += ZynqManager::PERIODIC;
	  }
	  found = mode.find("self");
	  if (found != std::string::npos) {
	    mode_to_set += ZynqManager::SELF;
	  }
	  found = mode.find("immediate");
	  if (found != std::string::npos) {
	    mode_to_set += ZynqManager::IMMEDIATE;
	  }
	  found = mode.find("external");
	  if (found != std::string::npos) {
	    mode_to_set += ZynqManager::EXTERNAL;
	  }
	}
	else {
          std::cout << "Error: for -zynq option the mode could not be identified, use mecontrol -help to check the available modes" << std::endl;
	  return NULL;
	}
	this->CmdLine->zynq_mode = mode_to_set;
	
      }
      
      /* set zynq_mode_string */
      this->CmdLine->zynq_mode_string = mode;
      
    }
    else {
      std::cout << "Error: for -zynq option the mode could not be identified, use mecontrol -help to check the available modes" << std::endl;
      return NULL;
    }

  }
  if(cmdOptionExists("-test_zynq")){
    this->CmdLine->test_zynq_on = true;

    /* zynq test mode */
    const std::string &test_mode = getCmdOption("-test_zynq");
    if (!test_mode.empty()){
      if (test_mode == "none") {
	this->CmdLine->zynq_test_mode = ZynqManager::T_NONE;
      }
      else if (test_mode == "ecasic") {
	this->CmdLine->zynq_test_mode = ZynqManager::ECASIC;
      }
      else if (test_mode == "pmt") {
	this->CmdLine->zynq_test_mode = ZynqManager::PMT;
      }
      else if (test_mode == "pdm") {
	this->CmdLine->zynq_test_mode = ZynqManager::PDM;
      }
      else if (test_mode == "l1") {
	this->CmdLine->zynq_test_mode = ZynqManager::L1;
      }
      else if (test_mode == "l2") {
	this->CmdLine->zynq_test_mode = ZynqManager::L2;
      }
      else if (test_mode == "L3") {
	this->CmdLine->zynq_test_mode = ZynqManager::L3;
      }
      else {
	std::cout << "Error: for -test_zynq option the mode could not be identified, use mecontrol -help to check the available modes" << std::endl;
	return NULL;
      }
    }
    else {
      std::cout << "Error: for -test_zynq option the mode could not be identified, use mecontrol -help to check the available modes" << std::endl;
      return NULL;
    }

    /* also set Zynq mode to PERIODIC to enable data collecting */
    this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
    
  }
  if(cmdOptionExists("-keep_zynq_pkt")){
    this->CmdLine->keep_zynq_pkt = true;
  }
  if(cmdOptionExists("-check_status")){
    this->CmdLine->check_status = true;
  }

  /* comment to go in file header and filename */
   if(cmdOptionExists("-comment")){

     const std::string &comment_str = getCmdOption("-comment");
     if (!comment_str.empty()){
       this->CmdLine->comment = comment_str;

       /* parse string to be suitable for filename */
       /* replace spaces with underscores */
       std::string comment_fn_str = CpuTools::SpaceToUnderscore(comment_str);
       /* limit size */
       if (comment_fn_str.length() > FILENAME_COMMENT_LEN) {
       this->CmdLine->comment_fn = comment_fn_str.substr(FILENAME_COMMENT_LEN);
       }
       else {
	 this->CmdLine->comment_fn = "__" + comment_fn_str;
       }
     }
     else {
       std::cout << "Error: for -comment option a comment string must be provided" << std::endl;
       return NULL;
     }
   }
   
  /* get the arguments */
  /* dynode voltage */
  const std::string &dynode_voltage = getCmdOption("-dv");
  if (!dynode_voltage.empty()){

    /* if single number, make required string */
    if (dynode_voltage.length() <= 4) {
      int dv = std::stoi(dynode_voltage);
      if (dv < 0 || dv > 4096) {
	std::cout << "Error: for -dv option must provide a dynode voltage between 0 and 4096" << std::endl;
	return NULL;
      }
      this->CmdLine->hvps_dv_string = CpuTools::BuildStr("", ",", dv, N_EC);
    }
    /* else, set string to CmdLine */
    else {
      this->CmdLine->hvps_dv_string = dynode_voltage;
    }
 
  }
  
  const std::string &dynode_voltage_real = getCmdOption("-dvr");
  if (!dynode_voltage_real.empty()){

    /* if single number, make required string */
    if (dynode_voltage_real.length() <= 4) {
      int dvr = std::stoi(dynode_voltage_real);
      if (dvr < 0 || dvr > 1100) {
	std::cout << "Error: for -dvr option must provide a dynode voltage between 0 and 1100 V" << std::endl;
	return NULL;
      }

      /* convert V -> DAC */
      int converted_dv = (int)((float)HV_CONV_FAC * dvr);
      this->CmdLine->hvps_dv_string = CpuTools::BuildStr("", ",", converted_dv, N_EC);
    }
    /* else, set string to CmdLine */
    else {

      /* convert V -> DAC */
      std::vector<int> dvr_values = CpuTools::DelimStrToVec(dynode_voltage_real, ',', N_EC, false);
      std::for_each(dvr_values.begin(), dvr_values.end(),  [](int& d) { d = (int)((float)HV_CONV_FAC*d);});
      this->CmdLine->hvps_dv_string = CpuTools::BuildStrFromVec("", ",", dvr_values);
    }
    
  }

  
  /* high voltage dac */
  const std::string &asic_dac = getCmdOption("-asicdac");
  if (!asic_dac.empty()){
    this->CmdLine->asic_dac = std::stoi(asic_dac);
  }  

  /* high voltage dac (kept for compatibility) */
  const std::string &hv_dac = getCmdOption("-hvdac");
  if (!hv_dac.empty()){
    this->CmdLine->asic_dac = std::stoi(hv_dac);
  }  
  
  return this->CmdLine;
}

/**
 * print the help message detailing all accepted command line options
 * (for use with mecontrol -help) 
 */
int InputParser::PrintHelpMsg() {
  
  std::cout << "--------------------------------" << std::endl;
  std::cout << "Mini-EUSO command line interface" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  std::cout << std::endl;
  std::cout << "COMMAND: mecontrol" << std::endl;
  std::cout << "USAGE: mecontrol -option argument" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "DESCRIPTION" << std::endl;
  std::cout << "The default behaviour of the software is to run an infinite acquisition," << std::endl;
  std::cout << "without HV and with the DAC in the pedestal (DAC = 750)." << std::endl;
  std::cout << "For further information on the default configuration see the online documentation." << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" <<std::endl;
  std::cout << "Contact: capel.francesca@gmail.com" <<std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "GENERAL" << std::endl;
  std::cout << std::endl;
  std::cout << "-db:                 enter software test/debug mode" << std::endl;
  std::cout << "-log:                turn on logging (off by default)" << std::endl;
  std::cout << "-comment:            add a comment to the CPU file header and name (e.g. -comment \"your comment here\")" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "EXECUTE-AND-EXIT" << std::endl;
  std::cout << "These commands execute and exit without running an automated acquisition" << std::endl;
  std::cout << std::endl;
  std::cout << "-ver:                print the version info then exit" << std::endl;
  std::cout << "-lvps <MODE>:        switch a subsystem using the LVPS (<MODE> = \"on\" or \"off\") then exit the program" << std::endl;
  std::cout << "-subsystem <SUBSYS>: select subsystem to switch (<SUBSYS> = \"zynq\", \"cam\" or \"hk\"), \"zynq\" by default" << std::endl;
  std::cout << "-hvswitch <MODE>:    switch the high voltage (<MODE> = \"on\" or \"off\") then exit the program" << std::endl;
  std::cout << "NB: If switching on, you must also specify which ECs using -hv <EC_STRING>" << std::endl;
  std::cout << "-dv <X>:             provide the dynode voltage in DAC (<X> = 0 - 4096)" << std::endl;
  std::cout << "-dvr <X>:            provide the dynode voltage in VOLTS (<X> = 0 - 1100)" << std::endl;
  std::cout << "-asicdac <X>:        provide the HV DAC (<X> = 0 - 1000)" << std::endl;
  std::cout << "-check_status:       check the Zynq telnet connection, instrument status and HV status" << std::endl;
  std::cout << std::endl;
  std::cout << "Switching the LVPS manually" << std::endl;
  std::cout << "Example use case: mecontrol -lvps on -subsystem zynq" << std::endl;
  std::cout << "Note: the automated acquisition program switches subsystems on/off automatically as required" << std::endl;
  std::cout << std::endl;
  std::cout << "Switching the HVPS manually" << std::endl;
  std::cout << "Example use case: mecontrol -hvps on -dv 3200 -asicdac 500" << std::endl;
  std::cout << "Example use case: mecontrol -hvps off" << std::endl;
  std::cout << "Note: the automated acquisition program switches the HV on automatically as required," << std::endl;
  std::cout << "and switches it off automatically if the program is interrupted with CTRL-C" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "SUBSYSTEMS" << std::endl;
  std::cout << std::endl;
  std::cout << "-cam:                make an independent or simultaneous acquisition with the cameras" << std::endl;
  std::cout << "-cam -v:             make an independent or simultaneous acquisition with the cameras with verbose output" << std::endl;
  std::cout << "-therm:              make a simultaneous acquisition with the thermistors" << std::endl;
  std::cout << std::endl;
  std::cout << "Example use case: mecontrol -log -cam -therm" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "HIGH VOLTAGE" << std::endl;
  std::cout << std::endl;
  std::cout << "-hv <EC_STRING>:     run an automated acquisition with the HV on" << std::endl;
  std::cout << "-dv <X>:             provide the dynode voltage (<X> = 0 - 4096), default in ../config/dummy.conf" << std::endl;
  std::cout << "-dvr <X>:            provide the dynode voltage in VOLTS (<X> = 0 - 1100)" << std::endl;
  std::cout << "-asicdac <X>:        provide the HV DAC (<X> = 0 - 1000), default in ../config/dummy.conf" << std::endl;
  std::cout << "-check_status:       check the Zynq telnet connection, instrument status and HV status" << std::endl;
  std::cout << std::endl;
  std::cout << "Example use case: mecontrol -log -hv -dv 3200 -asicdac 500" << std::endl;
  std::cout << "Note: high voltage should switch off automatically if the program is interrupted with CTRL-C" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "SCURVE" << std::endl;
  std::cout << std::endl;
  std::cout << "-scurve:             take a single S-curve and exit" << std::endl; 
  std::cout << "-start:              start ASIC DAC for threshold scan (min = 0)" << std::endl; 
  std::cout << "-step:               step between consecutive ASIC DAC acquisitions" << std::endl; 
  std::cout << "-stop:               stop ASIC DAC for threshold scan (max = 1023)" << std::endl; 
  std::cout << "-acc:                number of GTU taken at each ASIC DAC step" << std::endl; 
 
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "ACQUISITION" << std::endl;
  std::cout << std::endl;
  std::cout << "-short <N>:          run a short acquisition of N CPU_PACKETs"<< std::endl;
  std::cout << "-zynq <MODE>:        use the Zynq acquisition mode (<MODE> = none, periodic, self, immediate, external, trigger, ta_trigger, default = periodic)" << std::endl;
  std::cout << "-test_zynq <MODE>:   use the Zynq test mode (<MODE> = none, ecasic, pmt, pdm, l1, l2, l3, default = pdm)" << std::endl;
  std::cout << "-keep_zynq_pkt:      keep the Zynq packets on FTP" << std::endl;
  std::cout << "-zynq_reboot:      reboot the Zynq for this acquisition" << std::endl;
  std::cout << std::endl;
  std::cout << "Example use case: mecontrol -log -test_zynq pdm -keep_zynq_pkt" << std::endl;
  std::cout << "Example use case: mecontrol -log -hv on -zynq trigger" << std::endl;
  std::cout << "NB: different zynq modes can be used together! E.g.: mecontrol -log -hv -zynq periodic,external" << std::endl;
  std::cout << "NB: different zynq test modes CANNOT be used together" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "NOTES" << std::endl;
  std::cout << "Execute-and-exit flags such as -db, -hv on/off and -lvps on/off can only be used one at a time" << std::endl;
  std::cout << "*ALWAYS CONFIRM THE HV IS SWITCHED OFF BEFORE ALLOWING LIGHT ON THE PDM*" << std::endl;
  std::cout << "to safely stop the program's exectution use CTRL-C" << std::endl;
 
  return 0;
}


/**
 * print the version of the instrument
 * (for use with mecontrol -ver) 
 */
int InputParser::PrintVersionInfo() {

  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "Mini-EUSO CPU SOFTWARE Version: " << VERSION << " Date: " << VERSION_DATE_STRING << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" << std::endl;
  std::cout << std::endl;

  return 0;
}

/**
 * Check that the inputs are part of the expected command line options.
 */
int InputParser::CheckInputs() {

  int error_count = 0;
  
  /* loop over inputs and check validity */
  for(auto &t : this->tokens) {
      
    /* only check -options */
    if (t.find('-') != std::string::npos) {
      
      bool allowed = std::find(this->allowed_tokens.begin(), this->allowed_tokens.end(), t)
	!= this->allowed_tokens.end();
      
      if (!allowed) {
	std::cout << "Error: command line option " << t << " is not recognised" << std::endl;
	error_count++;
      }
    }
    
  }

  
  return error_count;
}
