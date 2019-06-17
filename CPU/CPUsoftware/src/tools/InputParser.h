#ifndef _INPUT_PARSER_H
#define _INPUT_PARSER_H

#include <vector>
#include <algorithm>

#include "LvpsManager.h"
#include "ZynqManager.h"
#include "CpuTools.h"
#include "minieuso_data_format.h"

/*
 * maximum length of filename comments
 */
#define FILENAME_COMMENT_LEN 100
/*
 * HV conversion factor from actual voltage to dynode voltage dac
 */
#define HV_CONV_FAC (4096 / (2.44 * 466))

/**
 * struct to store the command line inputs 
 */
struct CmdLineInputs {

  /* command line options */
  bool help;
  bool ver;
  bool hvps_on;
  bool hvps_switch;
  bool debug_mode;
  bool log_on;
  bool trig_on;
  bool cam_on;
  bool therm_on;
  bool lvps_on;
  bool sc_on;
  bool single_run; 
  bool test_zynq_on;
  bool keep_zynq_pkt;
  bool check_status;
  bool zynq_reboot;
  bool hide_pixel;
  /* command line arguments */
  std::string hvps_dv_string;
  int asic_dac;
  LvpsManager::Status lvps_status;
  LvpsManager::SubSystem lvps_subsystem;
  ZynqManager::HvpsStatus hvps_status;
  std::string hvps_ec_string;
  //ZynqManager::InstrumentMode zynq_mode;
  uint8_t zynq_mode;
  ZynqManager::TestMode zynq_test_mode;
  bool cam_verbose;
  int acq_len;
  /* scurve */
  int sc_start;
  int sc_step;
  int sc_stop;
  int sc_acc;
  
  
  /* strings to store what is sent by user before parsing */
  std::string command_line_string;
  std::string zynq_mode_string;
  std::string comment;
  std::string comment_fn;
 
};

/**
 * empty string to be returned when nothing found by InputParser::getCmdOption()
 */
static const std::string empty_string("");
   
/**
 * class to parse command line input to program 
 */
class InputParser{
public:
  /**
   * stores the command line inputs
   */
  CmdLineInputs * CmdLine = new CmdLineInputs();
  
  InputParser(int & argc, char ** argv); 
  CmdLineInputs * ParseCmdLineInputs();
  int PrintVersionInfo();

  /**
   * get the command line options 
   * @param option flag to get option for 
   */
  const std::string getCmdOption(const std::string &option) const {

    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end()) {
      itr++;
      if (itr++ != this->tokens.end()) {
	itr--;
	return * itr;
      }
    }
    return empty_string;
  }
  
  /**
   * check if the command line option exists 
   * @param option check if this flag exists
   */
  bool cmdOptionExists(const std::string &option) const {

    bool found = std::find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();

    return found;
    
  }
  

private:
  /**
   * vector containing all the passed command line options
   */
  std::vector <std::string> tokens;
  /**
   * vector containting all allowed command line options
   */
  std::vector <std::string> allowed_tokens;
  
  int PrintHelpMsg();
  int CheckInputs();
};


#endif
/* _INPUT_PARSER_H */
