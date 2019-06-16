#include "CamManager.h"

/**
 * constructor.
 * initialisation of usb_num_storage_dev, n_relaunch attempt and launch_running
 */
CamManager::CamManager() {
  this->usb_num_storage_dev = 0;
  this->n_relaunch_attempt = 0;
  this->launch_running = false;
  this->nir_status = UNDEF;
  this->vis_status = UNDEF;

  /* get the camera serial numbers */
  this->ParseSerialNumbers();
  
}

/**
 * set verbose output for debugging 
 */
int CamManager::SetVerbose() {
  this->verbose = true;

  return 0;
}

/**
 * start camera acquisition and react to launch failure if necessary 
 */
int CamManager::StartAcquisition() {

  std::string output;
  const char * cam_cmd;
  
  /* start with both cameras set to ON */
  this->SetCamStatus(ON, ON);

  /* define the launch command */
  cam_cmd = this->DefineLaunchCmd();

  /* launch and check verbosity */
  if (this->verbose) {
    output = CpuTools::CommandToStr(cam_cmd);
    std::cout << output << std::endl;
  }
  else {
    output = CpuTools::CommandToStr(cam_cmd);   
  }

  /* check the launch output for "Error Trace:" */
  size_t found = output.find("Error Trace:");
  if (found != std::string::npos) {
    
    clog << "error: " << logstream::error << "camera launch failed" << std::endl;
    std::cout << "ERROR: camera launch failed" << std::endl;

    found = output.find("*** BUS RESET ***");
    if (found != std::string::npos) {

      std::cout << "ERROR: cameras BUS RESET" << std::endl;
    }
 
    /* look for a serial number */
    found = output.find(std::to_string(nir_serial));
    if (found != std::string::npos) {

      /* turn off NIR camera if its serial number is in error msg */
      this->nir_status = OFF;
    }
    found = output.find(std::to_string(vis_serial));
    if (found != std::string::npos) {

      /* turn off VIS camera if its serial number is in error msg */
      this->vis_status = OFF;
    }
    
    /* signal launch failure */
    this->launch_failed.set_value(true);
    return 1;	
  }

  /* check the launch output for "No camera was detected" */
  found = output.find("No camera was detected");
  if (found != std::string::npos) {
    
    clog << "error: " << logstream::error << "no cameras detected" << std::endl;
    std::cout << "ERROR: no cameras detected" << std::endl;

    /* signal launch failure */
    this->launch_failed.set_value(true);
    return 1;	
  }

  
 return 0;
}

/**
 * spawn thread to launch the camera software 
 * thread is joined if launch fails and detached if launch successful
 */
int CamManager::CollectData() {

 /* launch the camera software */
  std::cout << "starting camera acquisition..." << std::endl;
  clog << "info: " << logstream::info << "starting camera acquisition" << std::endl; 

  auto future = this->launch_failed.get_future();
  std::thread collect_cam_data (&CamManager::StartAcquisition, this);

  /* wait for launch to be marked as success/fail by CamManager::StartAcquisition() */
  auto status = future.wait_for(std::chrono::seconds(LAUNCH_TIMEOUT));   

  /* check if cameras failed to launch */
  if (status == std::future_status::ready) {

    /* wait for thread to join */
    collect_cam_data.join();
    
    /* clear the promise */
    this->launch_failed = std::promise<bool>();

    return 1;
  }
  else {

    /* if launch OK, detach thread */
    std::cout << "cameras launched sucessfully, now running in the background" << std::endl;
    this->launch_running = true;
    this->cam_thread_handle = collect_cam_data.native_handle(); 
    collect_cam_data.detach();
  }
  
  
  return 0;
}

/**
 * kill the camera thread. 
 * used when switching mode or shutting down.
 * the camera thread holds no locked resources.
 */
int CamManager::KillCamAcq() {

  std::string output;
  const char * ps_cmd;
  std::stringstream conv;
  
  clog << "info: " << logstream::info << "killing the camera acquisition, if possible" << std::endl;

  /* check camera launch successful, ie. thread runnning */
  if (this->launch_running) {

    /* look for multiplecam process */
    conv << "ps | grep multiplecam" << std::endl;
    ps_cmd = conv.str().c_str();
    output = CpuTools::CommnadToStr(ps_cmd);

    size_t found = output.find("multiplecam");
    if (found != std::string::npos) {

      /* kill the thread */
      /* justifiable as no locked resources */
      pthread_cancel(this->cam_thread_handle);      

    }
  }

  return 0;
}


/**
 * define the command to launch the cameras based on required situation (write_directory, NIR or VIS or both, etc)
 */
const char * CamManager::DefineLaunchCmd() {

  std::stringstream conv;
  std::string launch_cmd_str;
  const char * launch_cmd = "";

  /* check usb status */
  if (this->usb_num_storage_dev == 1 ||
      this->usb_num_storage_dev == 2) {

    /* set up the comand to launch cameras */
    conv << "(cd " << CAMERA_SOFTWARE_DIR << " && " << CAMERA_EXEC << " "
	 << CAMERA_SOFTWARE_DIR << " " << USB_WRITE_DIR << " " << this->nir_status << " " << this->vis_status
	 << ")" << std::endl;
    
  }
  else {

    /* set up the comand to launch cameras */
    conv << "(cd " << CAMERA_SOFTWARE_DIR << " && " << CAMERA_EXEC << " "
	 << CAMERA_SOFTWARE_DIR << " " << OTHER_WRITE_DIR << " " << this->nir_status << " " << this->vis_status
      	 << ")" << std::endl;  
  }

  /* convert stringstream to char * */
  launch_cmd_str = conv.str();
  launch_cmd = launch_cmd_str.c_str();
  
  return launch_cmd;
}


/**
 * set the camera status 
 * @param nir_status status to set the NIR camera
 * @param vis_status status to set the VIS camera
 */
void CamManager::SetCamStatus(CamStatus nir_status, CamStatus vis_status) {

  this->nir_status = nir_status;
  this->vis_status = vis_status;

  return;
}

/**
 * parses the multiplecam software cameras.ini file to get the NIR and VIS serial numbers
 * called by the CamManager constructor
 */
void CamManager::ParseSerialNumbers() {

  std::string line;
  std::ifstream file_to_parse;
  
  /* open the file defined in CamManager.h */
  file_to_parse.open(SERIAL_NUM_FILE);

  if (file_to_parse.is_open()) {

    clog << "info: " << logstream::info << "CamManager reading from the file" << SERIAL_NUM_FILE << std::endl; 

    /* read from the file one line at a time */
    while (getline(file_to_parse, line)) {

      std::istringstream in(line);
      std::string type;
      int serial_num;
      in >> serial_num;
      in >> type;

      if (type == "VIS") {
	this->vis_serial = serial_num;
      }
      if (type == "NIR") {
	this->nir_serial = serial_num; 
      }
    } /* while getline() */
    
    /* close the file */
    file_to_parse.close();
  } /* if file is open */
  
  return;
}
