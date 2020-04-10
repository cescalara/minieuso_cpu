#ifndef _CPU_TOOLS_H
#define _CPU_TOOLS_H

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>

#include <dirent.h>
#include <unistd.h>
#include <stdint.h>

#include "ZynqManager.h"
#include "minieuso_data_format.h"
/*
 * define maximum output length for use with CpuTools::CommandToStr()
 */
#define MAX_STR_LENGTH 2000

/**
 * class to provide useful functions to other parts of the software 
 */
class CpuTools {

public:
  
  CpuTools();
  static std::string CommandToStr(const char * cmd);
  static std::string IntToFixedLenStr(const int input, const int length);
  static void ClearFolder(const char * data_dir);
  static std::string SpaceToUnderscore(std::string);
  static bool PingConnect(const std::string & ip_address);
  static bool CheckFtp();
  static std::vector<int> DelimStrToVec(std::string input_string, char delim, uint8_t size, bool check_01);
  static std::string BuildStr(std::string stem, const std::string & sep, int val, int rep);
  static std::string BuildStrFromVec(std::string stem, std::string sep, std::vector<int> values);
  static std::streampos FileSize(std::string file_path);
  static uint32_t BuildCpuHeader(uint32_t type, uint32_t ver);
  static uint32_t BuildCpuTimeStamp();
  static std::vector<int> ReadMatrixDac10(std::string dac10_filename);
  
};

#endif
/* _CPU_TOOLS_H */
