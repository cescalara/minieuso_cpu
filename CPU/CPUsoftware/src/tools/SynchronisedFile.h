#ifndef _SYNCHRONISED_FILE_H
#define _SYNCHRONISED_FILE_H

#include <boost/crc.hpp>  
#include <mutex>
#include <memory>

#include "log.h"
#include "minieuso_data_format.h"
#include "ConfigManager.h"

/* for use with CRC checksum calculation */
/* redefine this to change to processing buffer size */
#ifndef PRIVATE_BUFFER_SIZE
#define PRIVATE_BUFFER_SIZE  1024
#endif

/* global objects */
std::streamsize const buffer_size = PRIVATE_BUFFER_SIZE;


/**
 * handles asynchronous writing to file from multiple threads 
 */
class SynchronisedFile {
public:
  /**
   * stores the path to the file
   */
  std::string path;

  SynchronisedFile(std::string path); 
  ~SynchronisedFile();

  /**
   * file writing options
   */
  enum WriteType : uint8_t {
    CONSTANT = 0,
    VARIABLE_D1 = 1,
    VARIABLE_D2 = 2,
    VARIABLE_HV = 3,
  };

  uint32_t Checksum();
  void Close();
  bool IsOpen();
  
  /**
   * template to allow different objects to be passed for writing
   */
  template <class GenericType>
  /**
   * write a payload to the SynchronisedFile
   * @param payload payload to be written to file
   * @param write_type the way in which to write to file 
   * @param ConfigOut configuration output used to get number of packets for Zynq D1/D2 (optional)
   */
  size_t Write(GenericType payload, WriteType write_type, std::shared_ptr<Config> ConfigOut = nullptr) {

    size_t check = 0;
    
    /* lock to one thread at a time */
    std::lock_guard<std::mutex> lock(_accessMutex);

    clog << "info: " << logstream::info << "writing to SynchronisedFile " << this->path << std::endl;
      
    /*  write the payload to the file */
    switch(write_type) {
    case CONSTANT:
   
      check = fwrite(payload, sizeof(*payload), 1, this->_ptr_to_file);
      if (check != 1) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
	return check;
      }
      
    break;
    case VARIABLE_D1:

      check = fwrite(payload, sizeof(*payload), ConfigOut->N1, this->_ptr_to_file);
      if (check != size_t(ConfigOut->N1)) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;

	/* DEBUG check why fwrite fails */
	std::cout << "FWRITE FAIL" << std::endl;
	std::cout << "check = " << check << std::endl;
	std::cout << "feof: " << feof(this->_ptr_to_file) << std::endl;
	std::cout << "ferror: " << ferror(this->_ptr_to_file) << std::endl;
  
	return check;
      }

      break;
    case VARIABLE_D2:
      
      check = fwrite(payload, sizeof(*payload), ConfigOut->N2, this->_ptr_to_file);
      if (check != size_t(ConfigOut->N2)) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
	
	/* DEBUG check why fwrite fails */
	std::cout << "FWRITE FAIL" << std::endl;
	std::cout << "check = " << check << std::endl;
	std::cout << "feof: " << feof(this->_ptr_to_file) << std::endl;
	std::cout << "ferror: " << ferror(this->_ptr_to_file) << std::endl;
  
	return check;
      }
      break;
    case VARIABLE_HV:

      check = fwrite(payload, sizeof(*payload), ConfigOut->hvps_log_len, this->_ptr_to_file);
      if (check != size_t(ConfigOut->hvps_log_len)) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
	
	/* DEBUG check why fwrite fails */
	std::cout << "FWRITE FAIL" << std::endl;
	std::cout << "check = " << check << std::endl;
	std::cout << "feof: " << feof(this->_ptr_to_file) << std::endl;
	std::cout << "ferror: " << ferror(this->_ptr_to_file) << std::endl;
  
	return check;
      }
      
      break;
   
    }

    return check;
  }

private:
  /**
   * protection for file access   
   */
  std::mutex _accessMutex;
  /**
   * pointer to the SynchronisedFile
   */
  FILE * _ptr_to_file = nullptr;
};

/**
 * handles access to the SynchronisedFile
 */
class Access {
public:
  Access(std::shared_ptr<SynchronisedFile> sf);

  /**
   * path to the SynchronisedFile
   */
  std::string path;
  uint32_t GetChecksum();
  void CloseSynchFile();

  /**
   * template to allow different objects to be passed for writing
   */
  template <class GenericType>
 /**
   * write a payload to the SynchronisedFile
   * @param payload payload to be written to file
   * @param write_type the way in which to write to file 
   * @param ConfigOut configuration output used to get number of packets for Zynq D1/D2 (optional)
   */
  void WriteToSynchFile(GenericType payload, SynchronisedFile::WriteType write_type, std::shared_ptr<Config> ConfigOut = nullptr) {
    /* call write to file */
    this->_sf->Write(payload, write_type, ConfigOut);
  }
  
private:
  /**
   * pointer to the SynchronisedFile
   */
  std::shared_ptr<SynchronisedFile> _sf;
};

#endif
/* _SYNCHRONISED_FILE_H */
