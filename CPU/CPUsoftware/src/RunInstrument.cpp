#include "RunInstrument.h"

/* default constructor */
RunInstrument::RunInstrument(CmdLineInputs * CmdLine) {
  this->CmdLine = CmdLine;
  this->current_mode = RunInstrument::UNDEF;
}

/* define start-up procedure upon switch-on */
int RunInstrument::StartUp() {

  /* check the log level */
  if (this->CmdLine->log_on == true) {
    clog.change_log_level(logstream::all);
  }
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;

  /* reload and parse the configuration file */
  std::string config_file = config_dir + "/dummy.conf";
  std::string config_file_local = config_dir + "/dummy_local.conf";
  ConfigManager CfManager(config_file, config_file_local);
  this->ConfigOut = CfManager.Configure();

  /* check for command line override to config */
  if (this->CmdLine->dv != -1) {
    this->ConfigOut->dynode_voltage = this->CmdLine->dv;
  }
  if (this->CmdLine->hvdac != -1) {
    this->ConfigOut->dac_level = this->CmdLine->hvdac;
  }
  
  /* turn on all systems */
  std::cout << "switching on all systems..." << std::endl;
  if (this->CmdLine->cam_on ==true) {
    this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  }
  this->Lvps.SwitchOn(LvpsManager::HK);
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);

  /* wait for boot */
  std::cout << "waiting for boot..." << std::endl;
  sleep(BOOT_TIME);
  
  /* test the connection to the zynq board */
  this->ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  this->ZqManager.InstStatus();
  this->ZqManager.HvpsStatus();

  return 0;
}

/* start running the instrument according to specifications */
int RunInstrument::Start() {

  /* check operational mode */
  /* add mode switching here */

  /* start data acquisition */
  /* build on acq_run from mecontrol */
  return 0;
}