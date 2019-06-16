#include "CpuTools.h"

/**
 * default constructor 
 */
CpuTools::CpuTools() {

}


/**
 * function to run command and pass stdout to a string 
 * @param cmd the command to send using popen
 * NB: maximum size of returned string is limited to MAX_STR_LENGTH
 * defined in CpuTools.h
 */
std::string CpuTools::CommandToStr(const char * cmd) {
  const int buf_size = 512;
  std::array<char, buf_size> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);

  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), buf_size, pipe.get()) != nullptr) {
      /* stop if result over a certain length */
      if (result.size() < MAX_STR_LENGTH) {
	result += buffer.data();
      }
    }
  }

  return result;
}

/**
 * function to convert int to a fixed length string 
 */
std::string CpuTools::IntToFixedLenStr(const int input, const int length)
{
    std::ostringstream ostr;

    if (input < 0)
        ostr << '-';

    ostr << std::setfill('0') << std::setw(length) << (input < 0 ? -input : input);

    return ostr.str();
}


/**
 * function to clear a directory  
 */
void CpuTools::ClearFolder(const char * data_dir) {

  DIR * theFolder = opendir(data_dir);
  struct dirent * next_file;
  char filepath[256];   
  while ((next_file = readdir(theFolder)) != NULL) {
    sprintf(filepath, "%s/%s", data_dir, next_file->d_name);
    remove(filepath);
  }
  closedir(theFolder);    
}


/**
 * replaces spaces in a std::string with underscores
 */
std::string CpuTools::SpaceToUnderscore(std::string text) {

  for(size_t i = 0; i < text.length(); i++) {
    if(text[i] == ' ') {
	text[i] = '_';
    }
  }
  
  return text;
}

/**
 * check if you can ping an IP address
 * @param ip_adress the IP address as a string
 */
bool CpuTools::PingConnect(std::string ip_address) {

  bool is_connected = false;

  /* define ping for single packet with 1 sec timeout */
  std::string cmd = "ping " + ip_address + " -c 10 -i 0.001 -w 1";
  std::string output = CommandToStr(cmd.c_str());

  /* debug */
  std::cout << output << std::endl;
  
  /* look for successful output */
  size_t found = output.find("10 packets transmitted, 10 received, 0% packet loss");
  if (found != std::string::npos) {
    is_connected = true;
  }
  
  return is_connected;
}

/**
 * check if FTP server is up
 */
bool CpuTools::CheckFtp() {

  bool ftp = false;
  std::string cmd = "netstat -a --numeric-hosts | grep ftp";
  std::string output = CommandToStr(cmd.c_str());

  size_t found = output.find(":ftp");
  if (found != std::string::npos) {
    ftp = true;
  }

  return ftp;
}


/**
 * convert comma delimited string of ints to vector of ints
 * @param input_string string to convert
 * @param size the number of ints expected
 * @param check_01 set to 0 if input int is not 0 or 1
 */
std::vector<int> CpuTools::DelimStrToVec(std::string input_string, char delim, uint8_t size, bool check_01) {

  std::vector<int> vect;
  std::stringstream ss(input_string);
  int val;

  /* read until end of string or vector has reached size */
  while (ss >> val && vect.size() < size) {

    /* block non 0/1 values if required */
    if (check_01){
      if (val != 0 && val != 1) {
	val = 0;
      }
    }
    /* add value to vector */
    vect.push_back(val);
    /* ignore commas */
    if (ss.peek() == delim) {
      ss.ignore();
    }
  }  
  
  return vect;
}

/**
 * build strings with repetetive ints following a stem
 * useful for ZynqManager telnet interface
 * @param stem the start of the string
 * @param sep the delimiter
 * @param val the int value to be repeated
 * @param number of times to repeat val
 */
std::string CpuTools::BuildStr(std::string stem, std::string sep, int val, int rep) {

  std::string output_string;
  std::stringstream ss;
  int i = 0;

  if (!stem.empty()) {
    ss << stem;
  }
  for (i = 0; i < rep; i++) {
    if (i = 0 && stem.empty()) {
      ss << val;
    }
    else {
      ss << sep;
      ss << val;
    }
  }
  ss << "\n";
  output_string = ss.str();
  
  return output_string; 
}

/**
 * like BuildStr() but for a vector argument
 * useful for ZynqManager telnet interface
 * @param stem the start of the string
 * @param sep the delimiter
 * @param values vector of int values
 */
std::string CpuTools::BuildStrFromVec(std::string stem, std::string sep, std::vector<int> values) {

  std::string output_string;
  std::stringstream ss;
  uint8_t i = 0;

  ss << stem;
  for (i = 0; i < values.size(); i++) {
    ss << sep;
    ss << values[i];
  }
  ss << "\n";
  output_string = ss.str();

  return output_string; 
}

/**
 * get the file size of an ifstream
 * @param file_path path to the file
 */
std::streampos CpuTools::FileSize(std::string file_path){

    std::streampos fsize = 0;
    std::ifstream file(file_path, std::ios::binary );

    fsize = file.tellg();
    file.seekg(0, std::ios::end);
    fsize = file.tellg() - fsize;
    file.close();

    return fsize;
}

/**
 * build the cpu file header
 * @param type header tag of the file type
 * @param ver header tag of the file type version
 */
uint32_t CpuTools::BuildCpuHeader(uint32_t type, uint32_t ver) {

  uint32_t header;
  header =  (((type)<<24) | (INSTRUMENT_ME_PDM<<16) | ((type)<<8) | (ver));
 
  return header;
}

/**
 * build the cpu timestamp 
 * simple UNIX timestamp
 */
uint32_t CpuTools::BuildCpuTimeStamp() {

  uint32_t timestamp = time(NULL);

  return timestamp;
}
