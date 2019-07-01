#include "SynchronisedFile.h"

/**
 * constructor.
 * @param path path to the SynchronisedFile to be created 
 */
SynchronisedFile::SynchronisedFile(std::string path) {

  this->path = path;
  const char * file_name = path.c_str();
  
  /* open file for appending */
  this->_ptr_to_file = fopen(file_name, "ab");
  if (!this->_ptr_to_file) {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
    std::cout << "ERROR: cannot open the file " << this->path << std::endl;
  }

}

/**
 * destructor 
 * closes the SynchronisedFile
 */
SynchronisedFile::~SynchronisedFile() {

  /* close the file */
  fclose(this->_ptr_to_file);
  this->_ptr_to_file = nullptr;

}

/**
 * Check if the SynchronisedFile is open.
 */
bool SynchronisedFile::IsOpen() {

  bool is_open;

  if (this->_ptr_to_file) {
    is_open = true;
  }
  else {
    is_open = false;
  }
  
  return is_open;
}

/**
 * calculate the CRC checksum and append to file
 */
uint32_t SynchronisedFile::Checksum() {

  /* lock to one thread at a time */
  std::lock_guard<std::mutex> lock(_accessMutex);

  /* close the file */
  fclose(this->_ptr_to_file);
  
  /* calculate the CRC */
  boost::crc_32_type crc_result;
  std::ifstream ifs(this->path, std::ios_base::binary);	
  if(ifs) {
    do {
      char buffer[buffer_size];
      ifs.read(buffer, buffer_size);
      crc_result.process_bytes(buffer, ifs.gcount());
    } while (ifs);
  }
  else {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
    return 1;
  }
  std::cout << std::hex << std::uppercase << "CRC = " << crc_result.checksum() << std::endl;
  clog << "info: " << logstream::info << "CRC for " << this->path << " = "
       << std::hex << std::uppercase << crc_result.checksum() << std::endl;

  /* reopen the file for further appending */
  const char * file_name = this->path.c_str();
  this->_ptr_to_file = fopen(file_name, "ab");
  if (!this->_ptr_to_file) {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
  }
  
  return crc_result.checksum();
}

/**
 * close the SynchronisedFile
 */
void SynchronisedFile::Close() {

  /* close the file */
  fclose(this->_ptr_to_file);
}

/**
 * constructor.
 * @param sf pointer to the Synchronisedfile to be accessed  
 */
Access::Access(std::shared_ptr<SynchronisedFile> sf) {
  this->_sf = sf;
  this->path = sf->path;
}

/**
 * get the checksum of the SynchronisedFile accessed
 */
uint32_t Access::GetChecksum() {

  uint32_t checksum =_sf->Checksum();
  return checksum;
}

/**
 * close the SynchronisedFile accessed
 */
void Access::CloseSynchFile() {

  this->_sf->Close();
}
  
