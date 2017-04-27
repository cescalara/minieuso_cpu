/*-------------------------------
                                 
TEST CPU PROGRAM                 
V1.1: April 2017                 
                                 
Full PDM data acquisition chain	 
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "globals.h"

int main(void) {

  /* definitions */
  Config ConfigOut;

  /*----------*/
  /* start-up */
  printf("TEST CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);

  /* create the log file */
  std::ofstream log_file(log_name,std::ios::out);
  logstream clog(log_file, logstream::all);
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;
 
  /* reload and parse the configuration file */
  std::string config_file = "../config/dummy.conf";
  std::string config_file_local = "../config/dummy_local.conf";
  ConfigOut = Configure(config_file, config_file_local);

  /* test the connection to the zynq board */
  CheckTelnet(ZYNQ_IP, TELNET_PORT);
  
  /* check the instrument and HV status */
  InstStatus();
  HvpsStatus();

  /*---------------------*/
  /* A typical 40min run */
  printf("Starting acquisition run");
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

  /* enable signal handling */
  signal(SIGINT, SignalHandler);  
  
  /* turn on the HV */
  HvpsTurnon(ConfigOut.cathode_voltage, ConfigOut.dynode_voltage);

  /* take an scurve */
  Scurve(ConfigOut.scurve_start, ConfigOut.scurve_step, ConfigOut.scurve_stop, ConfigOut.scurve_acc);

  /* set the DAC level */
  SetDac(ConfigOut.dac_level);
  
  /* start the triggered acquisition */
  DataAcquisitionStart();

  /* wait for the stop signal */
  while (1) {
    printf("Acquiring data...\n");
  }
  
  return 0; 
}


  