#include "RunInstrument.h"

std::atomic<bool> signal_shutdown{false};

/**
 * the constructor
 * @param CmdLine is a struct storing command line inputs
 * parsed with the InputParser class
 */
RunInstrument::RunInstrument(CmdLineInputs * CmdLine) {
  this->CmdLine = CmdLine;

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    this->current_inst_mode = RunInstrument::INST_UNDEF;
  } /* relase mutex */
  this->current_acq_mode = RunInstrument::ACQ_UNDEF;

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    this->_stop = false;
  } /* relase mutex */

}

/**
 * function to handle interrupt signals to the program
 */
void RunInstrument::SignalHandler(int signum) {

  std::cout << "STOPPING INSTRUMENT" << std::endl;

  /* signal to main program */
  signal_shutdown.store(true);

}


/**
 * function to switch a desired subsystem on/off
 * as specified by the object's CmdLine struct
 */
int RunInstrument::LvpsSwitch() {

  switch (this->CmdLine->lvps_status) {
    case LvpsManager::ON:

      switch (this->CmdLine->lvps_subsystem) {
      case LvpsManager::ZYNQ:
	std::cout << "Switching ON the ZYNQ" << std::endl;
	break;
      case LvpsManager::CAMERAS:
	std::cout << "Switching ON the CAMERAS" << std::endl;
	break;
      case LvpsManager::HK:
	std::cout << "Switching ON the HK" << std::endl;
	break;
      }
      this->Lvps.SwitchOn(this->CmdLine->lvps_subsystem);
      break;

    case LvpsManager::OFF:
      switch (this->CmdLine->lvps_subsystem) {
      case LvpsManager::ZYNQ:
	std::cout << "Switching OFF the ZYNQ" << std::endl;
	break;
      case LvpsManager::CAMERAS:
	std::cout << "Switching OFF the CAMERAS" << std::endl;
	break;
      case LvpsManager::HK:
	std::cout << "Switching OFF the HK" << std::endl;
	break;
      }
      this->Lvps.SwitchOff(this->CmdLine->lvps_subsystem);
      break;

    case LvpsManager::UNDEF:
      std::cout << "Error: Cannot switch subsystem, on/off undefined" << std::endl;
      break;
    }

  return 0;
}


/**
 * function to switch on the HVPS
 * if required by the object's CmdLine struct
 */
int RunInstrument::HvpsSwitch() {

  switch (this->CmdLine->hvps_status) {
  case ZynqManager::ON:
    std::cout << "Switching ON the HVPS" << std::endl;
    {
      std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
      this->Zynq.HvpsTurnOn(this->ConfigOut->cathode_voltage,
			    this->ConfigOut->dynode_voltage_string,
			    this->CmdLine->hvps_ec_string);
    }
    break;
  case ZynqManager::OFF:
    std::cout << "Switching OFF the HVPS" << std::endl;
    {
      std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
      this->Zynq.HvpsTurnOff();
      this->Zynq.SetDac(0);
      break;
    }
  case ZynqManager::UNDEF:
    std::cout << "Error: Cannot switch subsystem, on/off undefined" << std::endl;
    break;
  }
return 0;
}


/**
 * function to check the current instrument and HV status
 * uses the ZynqManager class member functions
 */
int RunInstrument::CheckStatus() {

  /* test the connection to the zynq board */
  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    this->Zynq.CheckConnect();

    if (this->Zynq.telnet_connected) {
      /* check the instrument and HV status */
      this->Zynq.GetInstStatus();
      this->Zynq.GetHvpsStatus();
    }

    else {
      std::cout << "ERROR: Zynq cannot reach Mini-EUSO over telnet" << std::endl;
      std::cout << "first try to ping 192.168.7.10 then try again" << std::endl;
    }
  }

  return 0;
}


/**
 * function to run quick debug tests of the subsystems
 */
int RunInstrument::DebugMode() {

  /* run through main subsystems for easy debugging */

  std::cout << "-----------------------------" << std::endl;
  std::cout << "Mini-EUSO software debug mode" << std::endl;
  std::cout << "-----------------------------" << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" << std::endl;
  std::cout << std::endl;
  std::cout << "running checks of all subsystems..." <<std::endl;
  std::cout << std::endl;
  
  /*
  std::cout << "USB" << std::endl;
  int num_usb_storage = this->Usb.LookupUsbStorage();
  std::cout << "there are " << num_usb_storage << " USB storage devices connected" << std::endl;
  this->Cam.usb_num_storage_dev = num_usb_storage;
  std::cout << std::endl;

  std::cout << "LVPS" << std::endl;
  std::cout << "switching on all subsystems... " << std::endl;
  std::cout << "cameras ON " << std::endl;
  this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  sleep(1);
  std::cout << "HK ON" << std::endl;
  this->Lvps.SwitchOn(LvpsManager::HK);
  sleep(1);
  std::cout << "Zynq ON" << std::endl;
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);
  sleep(1);
  std::cout << std::endl;
  
  std::cout << "ANALOG" << std::endl;
  std::cout << "running an acquisition..." << std::endl;
  this->Daq.Analog->GetLightLevel(this->ConfigOut);
  auto light_level = this->Daq.Analog->ReadLightLevel();
  int i = 0;
  for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
    std::cout << "photodiode channel " << i << ": " << light_level->photodiode_data[i] << std::endl;
  }
  float avg_sipm = 0;
  for (i = 0; i < N_CHANNELS_SIPM; i++) {
    avg_sipm += light_level->sipm_data[i];
  }
  avg_sipm = avg_sipm/N_CHANNELS_SIPM;
  std::cout << "SIPM 64 channel average: " << avg_sipm << std::endl;
  std::cout << "SIPM single channel: " << light_level->sipm_single << std::endl;
  std::cout << std::endl;

  this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  std::cout << "CAMERAS" << std::endl;
  std::cout << "running an acquisition..." << std::endl;
  this->CmdLine->cam_on = true;
  this->CmdLine->cam_verbose = true;
  this->LaunchCam();
  sleep(2);
  std::cout << "stopping acquisition... ";
  this->Cam.KillCamAcq();
  std::cout << "done!" << std::endl;
  std::cout << std::endl;
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);

  std::cout << "ZYNQ" << std::endl;
  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    this->Zynq.CheckConnect();
    if (this->Zynq.telnet_connected) {
      this->Zynq.GetInstStatus();
      this->Zynq.GetHvpsStatus();
    }
    else {
      std::cout << "ERROR: Zynq cannot reach Mini-EUSO over telnet" << std::endl;
      std::cout << "first try to ping 192.168.7.10 then try again" << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "switching off all subsystems... " << std::endl;
  std::cout << "cameras OFF" << std::endl;
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  sleep(1);
  std::cout << "HK OFF" << std::endl;
  this->Lvps.SwitchOff(LvpsManager::HK);
  sleep(1);
  std::cout << "Zynq OFF " << std::endl;
  this->Lvps.SwitchOff(LvpsManager::ZYNQ);
  std::cout << "done!" << std::endl;
  */

  /* check the available disk space */
  const char * cmd = "df -h";
  std::string output = CpuTools::CommandToStr(cmd);	

  std::cout << "Checking disk space: " << std::endl;
  std::cout << output << std::endl;

  clog << "info: " << logstream::info << "Checking disk space:" << std::endl << output << std::endl;
 
  
  std::cout << "debug tests completed, exiting the program" << std::endl;

  return 0;
}


/**
 * set the instrument mod in a thread-safe way
 */
int RunInstrument::SetInstMode(InstrumentMode mode_to_set) {

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    this->current_inst_mode = mode_to_set;

    /* set the config to allow info to be passed around */
    this->ConfigOut->instrument_mode = mode_to_set;

  } /* release mutex */

  return 0;
}


/**
 * read the instrument mode in a thread-safe way
 */
RunInstrument::InstrumentMode RunInstrument::GetInstMode() {
  InstrumentMode current_inst_mode;

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    current_inst_mode = this->current_inst_mode;
  } /* release mutex */

  return current_inst_mode;
}


/**
 * initialise the instrument mode using the current light level status
 * light level is acquired using the AnalogManager to run an acquisition
 * from the photodiodes
 */
int RunInstrument::InitInstMode() {

  clog << "info: " << logstream::info << "setting the instrument mode" << std::endl;
  printf("info: setting the instrument mode \n");

  /* get the current light level */
  AnalogManager::LightLevelStatus current_lightlevel_status = this->Daq.Analog->CompareLightLevel(this->ConfigOut);

  /* make a decision */
  switch(current_lightlevel_status){

  case AnalogManager::LIGHT_ABOVE_DAY_THR:

    /* set to day mode */
    /* To notify isDay to an external program for zip purpose */
    this->isDay.open("/media/usb0/is_day.txt");
    this->isDay <<  "1";
    this->isDay.close();

    this->SetInstMode(RunInstrument::DAY);
    break;

  case AnalogManager::LIGHT_BELOW_NIGHT_THR:

    /* set to night mode */
    /* To notify isDay to an external program for zip purpose */
    this->isDay.open ("/media/usb0/is_day.txt");
    this->isDay <<  "2";
    this->isDay.close();

    this->SetInstMode(RunInstrument::NIGHT);
    break;

  case AnalogManager::LIGHT_UNDEF:

    if (GetInstMode() == INST_UNDEF){
      /* set to day mode to be safe */
      /* To notify isDay to an external program for zip purpose */
      this->isDay.open ("/media/usb0/is_day.txt");
      this->isDay <<  "3";
      this->isDay.close();
      
      this->SetInstMode(RunInstrument::DAY);
    }

    break;
  }

  return 0;
}


/**
 * run the start-up procedure
 * sets up logging, parses the config file and checks for command line
 * override to configured values
 */
int RunInstrument::StartUp() {

  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "Mini-EUSO CPU SOFTWARE Version: " << VERSION << " Date: " << VERSION_DATE_STRING << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" << std::endl;
  std::cout << std::endl;

  /* check the log level */
  if (this->CmdLine->log_on) {
    clog.change_log_level(logstream::all);
  }
  else {
   /* remove the log file */
    std::string cmd = "rm " + log_name;
    system(cmd.c_str());
  }
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;

  clog << "info: " << logstream::info << "Mini-EUSO CPU SOFTWARE Version: " << VERSION << "Date: " << VERSION_DATE_STRING << std::endl;
  
  /* reload and parse the configuration file */
  std::string config_dir(CONFIG_DIR);
  std::string conf_file_usb0 = "/media/usb0/dummy_usb.conf";
  std::string conf_file_usb1 = "/media/usb1/dummy_usb.conf";
  std::string conf_file_local = config_dir + "/dummy_local.conf";
  ConfigManager CfManager(conf_file_local, conf_file_usb0, conf_file_usb1);
  CfManager.Configure();

  /* check the configuration file has been parsed */
  if (!CfManager.IsParsed()) {
    /* exit with error */
    std::cout << "Error: configuration parsing failed" << std::endl;
    return 1;
  }

  this->ConfigOut = CfManager.ConfigOut;

  /* check for command line override to config */
  if (!this->CmdLine->hvps_dv_string.empty()) {
    this->ConfigOut->dynode_voltage_string = this->CmdLine->hvps_dv_string;
  }
  if (this->CmdLine->asic_dac != -1) {
    this->ConfigOut->dac_level = this->CmdLine->asic_dac;
  }
  if (this->CmdLine->sc_start != -1) {
    this->ConfigOut->scurve_start = this->CmdLine->sc_start;
  }
  if (this->CmdLine->sc_step != -1) {
    this->ConfigOut->scurve_step = this->CmdLine->sc_step;
  }
  if (this->CmdLine->sc_stop != -1) {
    this->ConfigOut->scurve_stop = this->CmdLine->sc_stop;
  }
  if (this->CmdLine->sc_acc != -1) {
    this->ConfigOut->scurve_acc = this->CmdLine->sc_acc;
  }

  //By Giammanco to switchoff the broken pixels
  if (this->CmdLine->hide_pixel == true) {
    this->Zynq.HidePixels();
  }


  /* print configuration parameters */
  printf("CONFIGURATION PARAMETERS\n");
  printf("CATHODE_VOLTAGE is %d\n", this->ConfigOut->cathode_voltage);
  printf("DYNODE_VOLTAGE is %s\n", this->ConfigOut->dynode_voltage_string.c_str());
  printf("SCURVE_START is %d\n", this->ConfigOut->scurve_start);
  printf("SCURVE_STEP is %d\n", this->ConfigOut->scurve_step);
  printf("SCURVE_STOP is %d\n", this->ConfigOut->scurve_stop);
  printf("SCURVE_ACC is %d\n", this->ConfigOut->scurve_acc);
  if (this->ConfigOut->dac_level == NO_DAC_SET) {
   printf("DAC_LEVEL will not be set by the software");
  }
  else {
    printf("DAC_LEVEL is %d\n", this->ConfigOut->dac_level);
  }
  printf("N1 is %d\n", this->ConfigOut->N1);
  printf("N2 is %d\n", this->ConfigOut->N2);
  printf("L2_N_BG is %d\n", this->ConfigOut->L2_N_BG);
  printf("L2_LOW_THRESH is %d\n", this->ConfigOut->L2_LOW_THRESH);
  printf("ARDUINO_WAIT_PERIOD is %d\n", this->ConfigOut->arduino_wait_period);
  printf("ANA_SENSOR_NUM is %d\n", this->ConfigOut->ana_sensor_num);
  printf("AVERAGE_DEPTH is %d\n", this->ConfigOut->average_depth);
  printf("DAY_LIGHT_THRESHOLD is %d\n", this->ConfigOut->day_light_threshold);
  printf("NIGHT_LIGHT_THRESHOLD is %d\n", this->ConfigOut->night_light_threshold);
  printf("LIGHT_POLL_TIME is %d\n", this->ConfigOut->light_poll_time);
  printf("LIGHT_ACQ_TIME is %d\n", this->ConfigOut->light_acq_time);
  printf("STATUS_PERIOD is %d\n", this->ConfigOut->status_period);
  printf("POWER_ON_DELAY is %d\n", this->ConfigOut->pwr_on_delay);
  printf("CAMERA_ON is %d\n", this->ConfigOut->camera_on);

  std::cout << std::endl;
  
  return 0;
}


/**
 * checks the subsystems are operating as required
 */
int RunInstrument::CheckSystems() {

  std::cout << "SUBSYSTEMS TO BE USED IN ACQUISITION" << std::endl;
  std::cout << "Zynq board" << std::endl;
  std::cout << "Analog board" << std::endl;

  if (this->CmdLine->hvps_on) {
    std::cout << "HVPS" << std::endl;
  }
  if (this->CmdLine->cam_on) {
    std::cout << "Cameras" << std::endl;
  }
  if (this->CmdLine->therm_on) {
    std::cout << "Thermistors" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "STARTING INSTRUMENT" << std::endl;

  /* first power off all systems, for a clean start */
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  this->Lvps.SwitchOff(LvpsManager::HK);
  if (this->CmdLine->zynq_reboot) {
    this->Lvps.SwitchOff(LvpsManager::ZYNQ);
  }

  /* turn on all systems */
  std::cout << "switching on all systems..." << std::endl;
  if (this->CmdLine->cam_on ==true) {
    this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  }
  this->Lvps.SwitchOn(LvpsManager::HK);
  sleep(this->ConfigOut->pwr_on_delay);
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);

  /* wait for boot */
  std::cout << "waiting for boot..." << std::endl;
  this->CheckStatus();

  /* check the number storage Usbs connected */
  std::cout << "there are " << (int)this->Usb.LookupUsbStorage() << " USB storage devices connected " << std::endl;
  this->Daq.usb_num_storage_dev = this->Usb.num_storage_dev;
  this->Cam.usb_num_storage_dev = this->Usb.num_storage_dev;


  /* check the available disk space */
  const char * cmd = "df -h";
  std::string output = CpuTools::CommandToStr(cmd);	

  std::cout << "Checking disk space: " << std::endl;
  std::cout << output << std::endl;

  clog << "info: " << logstream::info << "Checking disk space:" << std::endl << output << std::endl;
  
  /* initialise the instrument mode */
  InitInstMode();

  return 0;
}


/**
 * determines the AcquisitionMode from the program inputs
 */
int RunInstrument::SelectAcqOption() {

  /* select standard or scurve */
  if (this->CmdLine->sc_on) {
    this->current_acq_mode = SCURVE;
    this->ConfigOut->acquisition_mode = SCURVE;
  }
  else {
    this->current_acq_mode = STANDARD;
    this->ConfigOut->acquisition_mode = STANDARD;
  }

  /* select Zynq acquisition mode */
  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    this->Zynq.zynq_mode = this->CmdLine->zynq_mode;
    this->Zynq.test_mode = this->CmdLine->zynq_test_mode;
  }

  return 0;
}

/**
 * launches the camera acquisition in a robust way
 * makes use of the CamManager member functions to check
 * if the launch fails and reacts by trying to relaunch
 * up to 3 times
 */
int RunInstrument::LaunchCam() {
  size_t check = 0;

  this->Cam.n_relaunch_attempt = 0;

  /* set which cameras should be switched on */
  if (this->ConfigOut->camera_on == 11) {
    this->Cam.SetCamStatus(CamManager::ON, CamManager::ON);
  }
  else if (this->ConfigOut->camera_on == 10) {
    /* NIR only */
    this->Cam.SetCamStatus(CamManager::ON, CamManager::OFF);
  }
  else if (this->ConfigOut->camera_on == 1) {
    /* VIS only */
    this->Cam.SetCamStatus(CamManager::OFF, CamManager::ON);
  }
  else {
    /* if not set correctly, put both on */
    this->Cam.SetCamStatus(CamManager::ON, CamManager::ON);
  }
   
  /* launch cameras, if required */
  if (this->CmdLine->cam_on) {

    /* check verbosity */
    if (this->CmdLine->cam_verbose) {
      this->Cam.SetVerbose();
    }

    check = this->Cam.CollectData();

    /* react if launched with errors */
    while ((check != 0) &&
	(this->Cam.n_relaunch_attempt < N_TRY_RELAUNCH)) {

      std::cout << "Camera relaunch attempt " << this->Cam.n_relaunch_attempt << std::endl;
      clog << "info: " << logstream::info << "camera relaunch attempt no. " << this->Cam.n_relaunch_attempt << std::endl;

      std::cout << "Rebooting the cameras" << std::endl;
      clog << "info: " << logstream::info << "rebooting the cameras" << std::endl;

      /* reboot the cameras */
      this->Lvps.SwitchOff(LvpsManager::CAMERAS);
      sleep(1);
      this->Lvps.SwitchOff(LvpsManager::CAMERAS);
      sleep(1);

      std::cout << "Relaunching the cameras" << std::endl;
      clog << "info: " << logstream::info << "relaunching the cameras" << std::endl;

      /* relaunch */
      check = this->Cam.CollectData();
      this->Cam.n_relaunch_attempt++;
    }
    if (check != 0) {

      std::cout << "ERROR: cameras failed to relaunch" << std::endl;
      clog << "error: " << logstream::error << "cameras failed to relaunch" << std::endl;

    }
  }

  return 0;
}


/**
 * launches a background thread to monitor the instrument
 * runs RunInstrument::PollInstrument member function
 */
int RunInstrument::MonitorInstrument() {

  /* launch a thread to watch the photodiode measurements */
  std::thread instrument_monitor (&RunInstrument::PollInstrument, this);

  /* detach */
  instrument_monitor.detach();

  return 0;
}


/**
 * sends a stop signal to all processes in a thread-safe way
 */
int RunInstrument::SetStop() {

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    this->_stop = true;
  } /* release mutex */

  return 0;
}


/**
 * checks if a stop signal has been sent
 */
bool RunInstrument::CheckStop() {
  bool stop_status;

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    stop_status = this->_stop;
  } /* release mutex */

  return stop_status;
}

/**
 * instrument monitoring by checking the light level and shutdown signal
 */
int RunInstrument::PollInstrument() {

  /* different procedure for day and night */
  while (!signal_shutdown.load()) {

    switch(GetInstMode()) {

    case RunInstrument::NIGHT:

      sleep(this->ConfigOut->light_poll_time);

      /* check if the output of the analog acquisition is above day threshold */
      if (this->Daq.Analog->CompareLightLevel(this->ConfigOut) == AnalogManager::LIGHT_ABOVE_DAY_THR) {

	/* switch mode to DAY */
	printf("PollInst: from night to day\n");
	clog << "info: " << logstream::info << "PollInstrument: from night to day" << std::endl;
 
	/* To notify isDay to an external program for zip purpose */
	this->isDay.open ("/media/usb0/is_day.txt");
	this->isDay <<  "1";
	this->isDay.close();

	this->SetInstMode(RunInstrument::DAY);
	this->Daq.Notify();

      }
      break;

    case RunInstrument::DAY:

      sleep(this->ConfigOut->light_poll_time);

      /* check the output of analog acquisition is below night threshold */
      if (this->Daq.Analog->CompareLightLevel(this->ConfigOut) == AnalogManager::LIGHT_BELOW_NIGHT_THR) {

	/* switch mode to NIGHT */
	printf("\nPollInst: from day to night\n");
	clog << "info: " << logstream::info << "PollInstrument: from day to night" << std::endl;
 
	/* To notify isDay to an external program for zip purpose */
	this->isDay.open ("/media/usb0/is_day.txt");
	this->isDay<< "2";
	this->isDay.close();

	this->SetInstMode(RunInstrument::NIGHT);
	this->Data.Notify();

      }
      break;

    case RunInstrument::INST_UNDEF:

      std::cout << "ERROR: instrument mode is undefined" << std::endl;
      /* To notify isDay to an external program for zip purpose */
      this->isDay.open ("/media/usb0/is_day.txt");
      this->isDay<< "3";
      this->isDay.close();

      this->SetInstMode(RunInstrument::DAY);
      this->Daq.Notify();

      break;
    }

  } /* end loop when stop signal sent */

  SetStop();

  /* stop running threads */
  clog << "info: " << logstream::info << "stopping joinable threads..." << std::endl;
  std::cout << "stopping joinable threads..." << std::endl;

  switch(GetInstMode()) {
  case NIGHT:

    this->Daq.Notify();
    break;

  case DAY:

    this->Data.Notify();
    break;

  case INST_UNDEF:

    clog << "info: " << logstream::info << "instrument mode undefined, no threads to stop" << std::endl;
    std::cout << "instrument mode undefined, no threads to stop" << std::endl;
    break;

  }

  clog << "info: " << logstream::info << "exiting instrument monitoring thread..." << std::endl;
  std::cout << "exiting instrument monitoring thread..." << std::endl;
  return 0;
}

/**
 * launches a background thread to check and
 * print the instrument status
 */
int RunInstrument::StatusChecker() {

  /* launch a thread to check the status periodically */
  std::thread status_checker (&RunInstrument::RunningStatusCheck, this);

  /* detach */
  status_checker.detach();

  return 0;
}

/**
 * print the status to the screen every STATUS_PERIOD
 * seconds
 */
int RunInstrument::RunningStatusCheck() {

  /* wait for first update */
  sleep(10);

  while(!signal_shutdown.load()) {

    /* get the status */
    std::string zynq_status, hk_status, cam_status;
    std::string zynq_telnet_status, hv_status;
    int n_files_written = 0;

    /* LVPS powered systems */
    /*
    if (this->Lvps.Check(LvpsManager::ZYNQ)) {
      zynq_status = "ON";
    }
    else {
      zynq_status = "OFF";
    }
    if (this->Lvps.Check(LvpsManager::HK)) {
      hk_status = "ON";
    }
    else {
      hk_status = "OFF";
    }
    if (this->Lvps.Check(LvpsManager::CAMERAS)) {
      cam_status = "ON";
    }
    else {
      cam_status = "OFF";
    }
    */

    /*
    std::cout << "Subsystems power status" << std::endl;
    std::cout << "Zynq: " << zynq_status << std::endl;
    std::cout << "HK: " << hk_status << std::endl;
    std::cout << "Cameras" << cam_status << std::endl;
    std::cout << std::endl;
    */

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "STATUS UPDATE" << std::endl;

    /* telnet connection and HV */
    {
      std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
      this->Zynq.CheckConnect();

      std::cout << "Checking telnet connection..." << std::endl;
      if (this->Zynq.telnet_connected) {
	zynq_telnet_status = "CONNECTED";
	std::cout << "Telnet connection: " << zynq_telnet_status << std::endl;
	/* check the instrument and HV status */
	this->Zynq.GetInstStatus();
	this->Zynq.GetHvpsStatus();
      }
      else {
	zynq_telnet_status = "DISCONNECTED";
	std::cout << "Telnet connection: " << zynq_telnet_status << std::endl;
	std::cout << "Cannot display HV info from Zynq" << std::endl;
      }
    }

    /* data acquisition */
    {
      std::unique_lock<std::mutex> lock(this->Daq.m_nfiles);
      n_files_written = this->Daq.n_files_written;
    }
    std::cout << "No. of files written: " << n_files_written << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    /* wait until next status check */
    sleep(ConfigOut->status_period);

  }

  return 0;
}

/**
 * interface to the data acquisition
 * uses the DataAcquisition class member functions
 */
int RunInstrument::Acquisition() {

  std::cout << "starting acquisition run..." <<std::endl;
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

  /* clear the FTP server */
  CpuTools::ClearFolder(DATA_DIR);
  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    this->Zynq.InstrumentClean();
  }
  
  /* add acquisition with cameras if required */
  this->LaunchCam();


  /* set the ASIC DAC */
  if (this->ConfigOut->dac_level != NO_DAC_SET) {
    {
      std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
      this->Zynq.SetDac(this->ConfigOut->dac_level);
    }
  }

  /* select SCURVE or STANDARD acquisition */
  if (this->Zynq.telnet_connected) {
    SelectAcqOption();
    switch (this->current_acq_mode) {
    case SCURVE:

      /* take an scurve */
      Daq.CollectSc(&this->Zynq, this->ConfigOut, this->CmdLine);

      break;
    case STANDARD:

      /* start data acquisition */
      this->Daq.CollectData(&this->Zynq, this->ConfigOut, this->CmdLine);

      break;
    case ACQ_UNDEF:
      clog << "error: " << logstream::error << "RunInstrument AcquisitionMode is undefined" << std::endl;
      std::cout << "Error: RunInstrument AcquisitionMode is undefined" << std::endl;
    }
  }

  /* reached for SCURVE acq and instrument mode switch or stop */
  if (this->CmdLine->cam_on) {
    this->Cam.KillCamAcq();
  }

  return 0;
}


/**
 * night time operational procedure
 * does not return until all night processes have joined
 */
int RunInstrument::NightOperations() {

  /* check scurve not already completed */
  if (this->Daq.IsScurveDone()) {
    return 0;
  }

  clog << "info: " << logstream::info << "entering NIGHT mode" << std::endl;
  std::cout << "entering NIGHT mode..." << std::endl;

  /* reset mode switching */
  this->Daq.Reset();

  /* set the HV as required */
  if (this->CmdLine->hvps_on) {
    this->ConfigOut->hv_on = true;
    this->CmdLine->hvps_status = ZynqManager::ON;
    HvpsSwitch();
  }

  /* start data acquisition */
  /* acquisition runs until signal to switch mode */
  Acquisition();

  /* turn off HV */
  if (this->Zynq.telnet_connected) {
    this->CmdLine->hvps_status = ZynqManager::OFF;
    HvpsSwitch();
  }

  /* reboot the Zynq */
  clog << "info: " << logstream::info << "rebooting the Zynq system" << std::endl;
  std::cout << "rebooting the Zynq system..." << std::endl;
  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    this->Zynq.Reboot();
  }

  /* wait for telnet connection */
  clog << "info: " << logstream::info << "waiting for boot" << std::endl;
  std::cout << "waiting for boot..." << std::endl;
  this->CheckStatus();

  /* set hidden pixels */
  if (this->CmdLine->hide_pixel == true) {
    {
      std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
      this->Zynq.HidePixels();
    }
  }

  return 0;
}


/**
 * day time operational procedure
 * does not returnuntil all day processes have joined
 */
int RunInstrument::DayOperations() {

  clog << "info: " << logstream::info << "entering DAY mode" << std::endl;
  std::cout << "entering DAY mode..." << std::endl;

  /* reset mode switching */
  this->Data.Reset();

  /* data reduction runs until signal to switch mode */
  this->Data.Start();

  return 0;
}


/**
 * shut down procedure
 * called when stop signal sent
 */
void RunInstrument::Stop() {

  /* kill detached threads */
  clog << "info: " << logstream::info << "stopping detached threads..." << std::endl;
  std::cout << "stopping detached threads..." << std::endl;
  this->Cam.KillCamAcq();

  /* USB backup disabled for now, plan to work with 1 USB */
  //this->Usb.KillDataBackup();

  /* turn off all subsystems */
  /* leave zynq on all the time, for now */
  //this->Lvps.SwitchOff(LvpsManager::ZYNQ);
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  this->Lvps.SwitchOff(LvpsManager::HK);

  return;
}


/**
 * start running the instrument according to specifications
 * checks for execute and exit functions,
 * then moves on to automated loop over mode switching
 */
void RunInstrument::Start() {

  /* check for execute-and-exit commands */
  if (this->CmdLine->lvps_on) {
    LvpsSwitch();
    return;
  }
  if (this->CmdLine->check_status) {   //Zynq check
    CheckStatus();
    return;
  }

  /* run start-up  */
  int check = this->StartUp();
  if (check !=0 ){
    return;
  }

  /* check for execute-and-exit commands which require config */
  if (this->CmdLine->hvps_switch) {
    HvpsSwitch();
    return;
  }
  else if (this->CmdLine->debug_mode) {
    DebugMode();
    return;
  }

  /* check systems and operational mode */
  this->CheckSystems();

  {
    std::unique_lock<std::mutex> lock(this->Zynq.m_zynq);
    if (!this->Zynq.telnet_connected) {
      std::cout << "no Zynq connection, exiting the program" << std::endl;
      return;
    }
  }

  /* launch data backup in background */
  /* disable for now, planning to work with a single USB system */
  //this->Usb.RunDataBackup();

  /* launch background process to monitor the instrument */
  this->MonitorInstrument();

  /* launch background process to run status checker */
  this->StatusChecker();

  /* enable signal handling */
  signal(SIGINT, SignalHandler);

  /* enter instrument mode */
  while (!CheckStop()) {
    switch(GetInstMode()) {


      /* NIGHT OPERATIONS */
      /*------------------*/
    case NIGHT:
      this->NightOperations();
      break;


      /* DAY OPERATIONS */
      /*----------------*/
    case DAY:
      this->DayOperations();
      break;

      /* UNDEFINED */
      /*-----------*/
    case INST_UNDEF:
      std::cout << "ERROR: instrument mode undefined, cannot start acquisition" << std::endl;

      break;
    } /* end switch statement */

  } /* end loop when stop signal sent */

  /* program shutdown */
  Stop();

  std::cout << "exiting the program..." << std::endl;
  return;
}
