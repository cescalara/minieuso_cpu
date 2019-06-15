#include "DataAcquisition.h"

/**
 * constructor 
 */
DataAcquisition::DataAcquisition() { 

  /* filename initialisation */
  this->cpu_main_file_name = "";
  this->cpu_sc_file_name = "";    
  this->cpu_hv_file_name = "";

  /* usb storage devices */
  this->usb_num_storage_dev = 0;

  /* number of files written */
  {
    std::unique_lock<std::mutex> lock(this->m_nfiles);     
    this->n_files_written = 0;
  }
  
  /* scurve acquisition */
  this->_scurve = false;
}
  
/** 
 * create cpu run file name
 * checks if usb connected to define the data storage path
 * @param run_type defines the file run type
 * @param ConfigOut the output of configuration parsing with ConfigManager 
 */
std::string DataAcquisition::CreateCpuRunName(RunType run_type, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {
  struct timeval tv;
  char cpu_file_name[MAX_FILENAME_LENGTH];
  std::string done_str(DONE_DIR);
  std::string usb_str(USB_MOUNTPOINT_0);
  std::string time_str;

  switch (run_type) {
  case CPU:
    time_str = "/CPU_RUN_MAIN__%Y_%m_%d__%H_%M_%S"
      + CmdLine->comment_fn + ".dat";
    break;
  case SC:

    /* check if HV switched on */
    if (ConfigOut->hv_on) {
      time_str = "/CPU_RUN_SC__%Y_%m_%d__%H_%M_%S__"
	+ std::to_string(ConfigOut->dynode_voltage) 
	+ CmdLine->comment_fn + ".dat";
    }
    else {
      time_str = "/CPU_RUN_SC__%Y_%m_%d__%H_%M_%S__noHV"
	+ CmdLine->comment_fn + ".dat";
    }
    
    break;
  case HV:
    time_str = "/CPU_RUN_HV__%Y_%m_%d__%H_%M_%S"
      + CmdLine->comment_fn + ".dat";
    break;
  }
  
  std::string cpu_str;

  /* write on USB directly if possible */
  if (this->usb_num_storage_dev == 1 || usb_num_storage_dev == 2) {
    cpu_str = usb_str + time_str;
  }
  /* else write in DONE_DIR */
  else {
    cpu_str = done_str + time_str;
  }

  const char * kCpuCh = cpu_str.c_str();
  gettimeofday(&tv ,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  strftime(cpu_file_name, sizeof(cpu_file_name), kCpuCh, now_tm);
 
  return cpu_file_name;
}

/**
 * build the cpu file info based on runtime settings
 * @param ConfigOut the configuration file parameters and settings from RunInstrument
 * @param CmdLine the command line parameters
 */
std::string DataAcquisition::BuildCpuFileInfo(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {
  /* for info string */
  std::string run_info_string;
  std::stringstream conv;

  /* for current time */
  struct timeval tv;
  const char * time_fmt = "%d/%m/%Y %H:%M";
  char time[20];
  
  gettimeofday(&tv ,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  strftime(time, sizeof(time), time_fmt, now_tm);
  
  std::string zynq_ver = ZynqManager::GetZynqVer();
  
  /* parse the runtime settings into the run_info_string */
  conv << "Experiment: " << INSTRUMENT << std::endl;
  conv << "Date (UTC): " << time << std::endl;
  conv << "Software version: " << VERSION << " " << VERSION_DATE_STRING << std::endl;
  conv << "Zynq firmware version: " << zynq_ver.c_str() << std::endl; 
  conv << "Zynq acquisition/trigger mode: " << CmdLine->zynq_mode_string.c_str() << std::endl;
  conv << "Instrument and acquisition mode (defined in RunInstrument.h): "
       << (int)ConfigOut->instrument_mode << " " << (int)ConfigOut->acquisition_mode << std::endl;
  conv << "Command line args: " << CmdLine->command_line_string.c_str() << std::endl;
  conv << "Comment: " << CmdLine->comment.c_str() << std::endl;
  
  run_info_string = conv.str();
  
  return run_info_string;
}

/**
 * make a cpu data file for a new run 
 * @param run_type defines the file run type
 * @param ConfigOut the output of configuration parsing with ConfigManager
 * sets up the synchonised file access and notifies the ThermManager object
 */
int DataAcquisition::CreateCpuRun(RunType run_type, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {

  CpuFileHeader * cpu_file_header = new CpuFileHeader();
  
  /* set the cpu file name */
  switch (run_type) {
  case CPU: 
    this->cpu_main_file_name = CreateCpuRunName(CPU, ConfigOut, CmdLine);
    clog << "info: " << logstream::info << "Set cpu_main_file_name to: " << cpu_main_file_name << std::endl;
    this->CpuFile = std::make_shared<SynchronisedFile>(this->cpu_main_file_name);
    cpu_file_header->header = CpuTools::BuildCpuHeader(CPU_FILE_TYPE, CPU_FILE_VER);
    break;
  case SC: 
    this->cpu_sc_file_name = CreateCpuRunName(SC, ConfigOut, CmdLine);
    clog << "info: " << logstream::info << "Set cpu_sc_file_name to: " << cpu_sc_file_name << std::endl;
    this->CpuFile = std::make_shared<SynchronisedFile>(this->cpu_sc_file_name);
    cpu_file_header->header = CpuTools::BuildCpuHeader(SC_FILE_TYPE, SC_FILE_VER);
    break;
  case HV:
    this->cpu_hv_file_name = CreateCpuRunName(HV, ConfigOut, CmdLine);
    clog << "info: " << logstream::info << "Set cpu_hv_file_name to: " << cpu_hv_file_name << std::endl;
    this->CpuFile = std::make_shared<SynchronisedFile>(this->cpu_hv_file_name);
    cpu_file_header->header = CpuTools::BuildCpuHeader(HV_FILE_TYPE, HV_FILE_VER);
    break;
  }
  this->RunAccess = new Access(this->CpuFile);

  /* access for ThermManager */
  this->Analog->RunAccess = new Access(this->CpuFile);
    
  /* set up the cpu file structure */
  std::string run_info_string = BuildCpuFileInfo(ConfigOut, CmdLine);
  strncpy(cpu_file_header->run_info, run_info_string.c_str(), (size_t)run_info_string.length());

  if (CmdLine->single_run) {
    cpu_file_header->run_size = CmdLine->acq_len;
  }
  else {
    cpu_file_header->run_size = RUN_SIZE;
  }
 
  /* write to file */
  this->RunAccess->WriteToSynchFile<CpuFileHeader *>(cpu_file_header, SynchronisedFile::CONSTANT, ConfigOut);
  delete cpu_file_header;
  
  /* notify the ThermManager */
  /* will this only work the first time? */
  this->Analog->cpu_file_is_set = true;
  this->Analog->cond_var.notify_all();
  
  return 0;
}

/**
 * close the CPU file run and append CRC.
 * this closes the run and runs a CRC calculation which is 
 * the stored in the file trailer and appended
 */
int DataAcquisition::CloseCpuRun(RunType run_type) {

  CpuFileTrailer * cpu_file_trailer = new CpuFileTrailer();
  
  clog << "info: " << logstream::info << "closing the cpu run file called " << this->CpuFile->path << std::endl;
  
  /* set up the cpu file trailer */
  cpu_file_trailer->header = CpuTools::BuildCpuHeader(TRAILER_PACKET_TYPE, CPU_FILE_VER);
  cpu_file_trailer->run_size = RUN_SIZE;
  cpu_file_trailer->crc = this->RunAccess->GetChecksum(); 

  /* write to file */
  this->RunAccess->WriteToSynchFile<CpuFileTrailer *>(cpu_file_trailer, SynchronisedFile::CONSTANT);
  delete cpu_file_trailer;

  /* close the current SynchronisedFile */
  this->RunAccess->CloseSynchFile();

  /* update number of packets written */
  {
    std::unique_lock<std::mutex> lock(this->m_nfiles);     
    this->n_files_written++;
  }
  
  return 0;
}


/**
 * read out an scurve file into an SC_PACKET. 
 */
SC_PACKET * DataAcquisition::ScPktReadOut(std::string sc_file_name, std::shared_ptr<Config> ConfigOut) {

  FILE * ptr_scfile;
  SC_PACKET * sc_packet = new SC_PACKET();
  const char * kScFileName = sc_file_name.c_str();
  size_t check;

  clog << "info: " << logstream::info << "reading out the file " << sc_file_name << std::endl;
  
  ptr_scfile = fopen(kScFileName, "rb");
  if (!ptr_scfile) {
    clog << "error: " << logstream::error << "cannot open the file " << sc_file_name << std::endl;
    return NULL;
  }
  
  /* prepare the scurve packet */
  sc_packet->sc_packet_header.header = CpuTools::BuildCpuHeader(SC_PACKET_TYPE, SC_PACKET_VER);
  sc_packet->sc_packet_header.pkt_size = sizeof(SC_PACKET);
  sc_packet->sc_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();
  sc_packet->sc_start = ConfigOut->scurve_start;
  sc_packet->sc_step = ConfigOut->scurve_step;
  sc_packet->sc_stop = ConfigOut->scurve_stop;
  sc_packet->sc_acc = ConfigOut->scurve_acc;

  ptr_scfile = fopen(kScFileName, "rb");
  if (!ptr_scfile) {
    clog << "error: " << logstream::error << "cannot open the file " << sc_file_name << std::endl;
    return NULL;
  }

  /* read out the scurve data from the file */
  check = fread(&sc_packet->sc_data, sizeof(sc_packet->sc_data), 1, ptr_scfile);
  if (check != 1) {
    clog << "error: " << logstream::error << "fread from " << sc_file_name << " failed" << std::endl;

    /* debug */
    std::cout << "sizeof sc_packet->sc_data: " << (int)sizeof(sc_packet->sc_data) << std::endl;
    std::cout << "check: " << check << std::endl;
    std::cout << "feof: " << feof(ptr_scfile) << std::endl;
    std::cout << "ferror: " << ferror(ptr_scfile) << std::endl;
 
    return NULL;   
  }
 
  /* close the scurve file */
  fclose(ptr_scfile);
  
  return sc_packet;
}


/**
 * read out a hv file into an HV_PACKET 
 */
HV_PACKET * DataAcquisition::HvPktReadOut(std::string hv_file_name, std::shared_ptr<Config> ConfigOut) {

  HV_PACKET * hv_packet = new HV_PACKET();

  clog << "info: " << logstream::info << "reading out the file " << hv_file_name << std::endl;
    
  /* prepare the hv packet */
  hv_packet->hv_packet_header.header = CpuTools::BuildCpuHeader(HV_PACKET_TYPE, HV_PACKET_VER);
  hv_packet->hv_packet_header.pkt_size = sizeof(HV_PACKET);
  hv_packet->hv_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();

  /* check file size and set n_entries */
  uint32_t file_size = CpuTools::FileSize(hv_file_name);
  uint32_t n_entries =  file_size / sizeof(DATA_TYPE_HVPS_LOG_V1);
  if (n_entries > HVPS_LOG_SIZE_NRECORDS) {
    n_entries = HVPS_LOG_SIZE_NRECORDS;
  }
  hv_packet->N = n_entries;
  ConfigOut->hvps_log_len = n_entries;
  hv_packet->hvps_log.resize(n_entries);

  std::ifstream hv_file(hv_file_name, std::ios::binary);
  if (!hv_file) {
    clog << "error: " << logstream::error << "cannot open the file " << hv_file_name << std::endl;
    return NULL;
  }

  /* read out the zbh */
  hv_file.read(reinterpret_cast<char*>(&hv_packet->zbh), sizeof(ZynqBoardHeader));
  
  /* read out the hv data from the file */
  hv_file.read(reinterpret_cast<char*>(&hv_packet->hvps_log[0]), hv_packet->hvps_log.size() * sizeof(DATA_TYPE_HVPS_LOG_V1));
  if (!hv_file) {
    std::cout << "ERROR: fread from " << hv_file_name << " failed" << std::endl;
    clog << "error: " << logstream::error << "read from " << hv_file_name << " failed" << std::endl;
    return NULL;
  }
  
  /* close the hv file */
  hv_file.close();
  
  return hv_packet;
}


/**
 * read out a zynq data file into a ZYNQ_PACKET 
 */
ZYNQ_PACKET * DataAcquisition::ZynqPktReadOut(std::string zynq_file_name, std::shared_ptr<Config> ConfigOut) {

  FILE * ptr_zfile;
  ZYNQ_PACKET * zynq_packet = new ZYNQ_PACKET();
  Z_DATA_TYPE_SCI_L1_V2 * zynq_d1_packet_holder = new Z_DATA_TYPE_SCI_L1_V2();
  Z_DATA_TYPE_SCI_L2_V2 * zynq_d2_packet_holder = new Z_DATA_TYPE_SCI_L2_V2();

  const char * kZynqFileName = zynq_file_name.c_str();
  size_t check;

  clog << "info: " << logstream::info << "reading out the file " << zynq_file_name << std::endl;

  ptr_zfile = fopen(kZynqFileName, "rb");
  if (!ptr_zfile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return nullptr;
  }

  
  /* DEBUG */

  /*
  long fsize;
  fseek(ptr_zfile, 0L, SEEK_END);
  fsize = ftell(ptr_zfile);
  rewind(ptr_zfile);

  std::cout << "file size: " << fsize << std::endl;
  std::cout << "sizeof(*zynq_packet): " << sizeof(*zynq_packet) << std::endl;  
  std::cout << "zynq file name: " << zynq_file_name << std::endl;
  std::cout << "ptr_zfile: " << ptr_zfile << std::endl;
  std::cout << "zynq_packet: " << zynq_packet << std::endl;
  */
  
  /* write the number of N1 and N2 */
  zynq_packet->N1 = ConfigOut->N1;
  zynq_packet->N2 = ConfigOut->N2;

  /* read out a number of Zynq packets, depending on ConfigOut->N1 and ConfigOut->N2 */
  /* data level D1 */
  for (int i = 0; i < ConfigOut->N1; i++) {
    check = fread(zynq_d1_packet_holder, sizeof(*zynq_d1_packet_holder), 1, ptr_zfile);
    if (check != 1) {
      std::cout << "ERROR: fread from " << zynq_file_name << " failed" << std::endl;
      std::cout << "Check: " << check << std::endl;
      std::cout << "feof: " << feof(ptr_zfile) << std::endl;
      std::cout << "ferror: " << ferror(ptr_zfile) << std::endl;
  
      clog << "error: " << logstream::error << "fread from " << zynq_file_name << " failed" << std::endl;
      return nullptr;
    }
    zynq_packet->level1_data.push_back(*zynq_d1_packet_holder);
    zynq_packet->level1_data.shrink_to_fit();
  }
  /* data level D2 */
  for (int i = 0; i < ConfigOut->N2; i++) {
    check = fread(zynq_d2_packet_holder, sizeof(*zynq_d2_packet_holder), 1, ptr_zfile);
    if (check != 1) {
      std::cout << "ERROR: fread from " << zynq_file_name << " failed" << std::endl;
      std::cout << "Check: " << check << std::endl;
      std::cout << "feof: " << feof(ptr_zfile) << std::endl;
      std::cout << "ferror: " << ferror(ptr_zfile) << std::endl;
  
      clog << "error: " << logstream::error << "fread from " << zynq_file_name << " failed" << std::endl;

      return nullptr;
    }
    zynq_packet->level2_data.push_back(*zynq_d2_packet_holder);
    zynq_packet->level2_data.shrink_to_fit();
  } 
  /* data level D3 */
  check = fread(&zynq_packet->level3_data, sizeof(zynq_packet->level3_data), 1, ptr_zfile);
  if (check != 1) {
    std::cout << "ERROR: fread from " << zynq_file_name << " failed" << std::endl;
    std::cout << "Check: " << check << std::endl;
    std::cout << "feof: " << feof(ptr_zfile) << std::endl;
    std::cout << "ferror: " << ferror(ptr_zfile) << std::endl;
    clog << "error: " << logstream::error << "fread from " << zynq_file_name << " failed" << std::endl;
    return nullptr;
  }
  
  /* DEBUG */
  /*
  std::cout << "header D1 P0 = " << zynq_packet->level1_data[0].zbh.header << std::endl;
  std::cout << "payload_size D1 P0 = " << zynq_packet->level1_data[0].zbh.payload_size << std::endl;
  std::cout << "n_gtu D1 P0 = " << zynq_packet->level1_data[0].payload.ts.n_gtu << std::endl; 
  */
  
  /* close the zynq file */
  fclose(ptr_zfile);

  /* clean up */
  delete zynq_d1_packet_holder;
  delete zynq_d2_packet_holder;
  
  return zynq_packet;
}

/**
 * read out a HK_PACKET from the analog board 
 */
HK_PACKET * DataAcquisition::AnalogPktReadOut() {

  int i, j = 0;
  HK_PACKET * hk_packet = new HK_PACKET();
 
  /* collect data */
  auto light_level = this->Analog->ReadLightLevel();
  
  /* make the header of the hk packet and timestamp */
  hk_packet->hk_packet_header.header = CpuTools::BuildCpuHeader(HK_PACKET_TYPE, HK_PACKET_VER);
  hk_packet->hk_packet_header.pkt_size = sizeof(hk_packet);
  hk_packet->hk_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();

  /* read out the values */
  for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
    hk_packet->photodiode_data[i] = light_level->photodiode_data[i];
  }
  for (j = 0; j < N_CHANNELS_SIPM; j++) {
    hk_packet->sipm_data[j] = light_level->sipm_data[j];
  }
  hk_packet->sipm_single = light_level->sipm_single;
  
  return hk_packet;
}



/**
 * write the CPU_PACKET to the current CPU file 
 * @param zynq_packet the Zynq data acquired from the PDM
 * @param hk_packet the HK data acquired from the analog board
 * @param ConfigOut the configuration struct output of ConfigManager
 * asynchronous writes to the CPU file are handled with the SynchronisedFile class
 */
int DataAcquisition::WriteCpuPkt(ZYNQ_PACKET * zynq_packet, HK_PACKET * hk_packet, std::shared_ptr<Config> ConfigOut) {

  CPU_PACKET * cpu_packet = new CPU_PACKET();
  static unsigned int pkt_counter = 0;

  clog << "info: " << logstream::info << "writing new packet to " << this->cpu_main_file_name << std::endl;
  
  /* create the cpu packet header */
  cpu_packet->cpu_packet_header.header = CpuTools::BuildCpuHeader(CPU_PACKET_TYPE, CPU_PACKET_VER);
  cpu_packet->cpu_packet_header.pkt_size = sizeof(*cpu_packet);
  cpu_packet->cpu_packet_header.pkt_num = pkt_counter; 
  cpu_packet->cpu_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();
  hk_packet->hk_packet_header.pkt_num = pkt_counter;

  /* add the zynq and hk packets, checking for NULL */
  if (zynq_packet != nullptr) {
    cpu_packet->zynq_packet = * zynq_packet;
  }
  else {
    std::cout << "ERROR: Zynq packet is NULL, writing empty packet" << std::endl;
    clog << "error: " << logstream::error << "Zynq packet is NULL, writing empty packet" << std::endl;
  }
  delete zynq_packet;
 
  if (hk_packet != nullptr) {
    cpu_packet->hk_packet = * hk_packet;
  }
  else {
    std::cout << "ERROR: HK packet is NULL, writing empty packet" << std::endl;
    clog << "error: " << logstream::error << "HK packet is NULL, writing empty packet" << std::endl;    
  }
  delete hk_packet;
 

  /* write the CPU packet */
  //this->RunAccess->WriteToSynchFile<CPU_PACKET *>(cpu_packet, SynchronisedFile::VARIABLE, ConfigOut);
  /* cpu header */
  this->RunAccess->WriteToSynchFile<CpuPktHeader *>(&cpu_packet->cpu_packet_header,
						    SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<CpuTimeStamp *>(&cpu_packet->cpu_time,
						    SynchronisedFile::CONSTANT);
  /* hk packet */
  this->RunAccess->WriteToSynchFile<HK_PACKET *> (&cpu_packet->hk_packet,
						  SynchronisedFile::CONSTANT);
  /* zynq packet */
  this->RunAccess->WriteToSynchFile<uint8_t *>(&cpu_packet->zynq_packet.N1,
					       SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<uint8_t *>(&cpu_packet->zynq_packet.N2,
					       SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L1_V2 *>(&cpu_packet->zynq_packet.level1_data[0],
							     SynchronisedFile::VARIABLE_D1, ConfigOut);
  this->RunAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L2_V2 *>(&cpu_packet->zynq_packet.level2_data[0],
							      SynchronisedFile::VARIABLE_D2, ConfigOut);
  this->RunAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L3_V2 *>(&cpu_packet->zynq_packet.level3_data,
							      SynchronisedFile::CONSTANT);

  delete cpu_packet; 
  pkt_counter++;
  
  return 0;
}


/**
 * write the SC_PACKET to the CPU file
 * @param sc_packet the Scurve data from the Zynq board
 * asynchronous writes to the CPU file are handled with the SynchronisedFile class
 */
int DataAcquisition::WriteScPkt(SC_PACKET * sc_packet) {

  static unsigned int pkt_counter = 0;

  clog << "info: " << logstream::info << "writing new packet to " << this->cpu_sc_file_name << std::endl;

  /* write the SC packet */
  this->RunAccess->WriteToSynchFile<SC_PACKET *>(sc_packet, SynchronisedFile::CONSTANT);
  delete sc_packet;
  pkt_counter++;
  
  return 0;
}


/**
 * write the HV_PACKET to the CPU file 
 * @param hv_packet HV data from the Zynq board
 * asynchronous writes to the CPU file are handled with the SynchronisedFile class
 */
int DataAcquisition::WriteHvPkt(HV_PACKET * hv_packet, std::shared_ptr<Config> ConfigOut) {

  static unsigned int pkt_counter = 0;

  clog << "info: " << logstream::info << "writing new packet to " << this->cpu_hv_file_name << std::endl;

  /* write the HV packet */
  this->RunAccess->WriteToSynchFile<CpuPktHeader *>(&hv_packet->hv_packet_header,
						    SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<CpuTimeStamp *>(&hv_packet->hv_time,
						    SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<uint32_t *>(&hv_packet->N,
						SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<ZynqBoardHeader *>(&hv_packet->zbh,
						       SynchronisedFile::CONSTANT);
  this->RunAccess->WriteToSynchFile<DATA_TYPE_HVPS_LOG_V1 *>(&hv_packet->hvps_log[0],
							     SynchronisedFile::VARIABLE_HV, ConfigOut);

  delete hv_packet;
  pkt_counter++;
  
  return 0;
}


/**
 * Poll the lftp server on the Zynq to check for new files.
 * Also clears old files from lftp server before starting.
 * @param monitor If true, wait until the instrument mode switch is sent to 
 * stop polling the FTP server.
 */
void DataAcquisition::FtpPoll(bool monitor) {

  std::string output;

  const char * ftp_clear = "";
  std::string ftp_clear_str;

  const char * ftp_cmd = "";
  std::string ftp_cmd_str;

  std::stringstream conv1;
  std::stringstream conv2;

  /* clear the server */
  clog << "info: " << logstream::info << "clearing old files from FTP server" << std::endl;

  /* build the command */
  conv1 << "lftp -u minieusouser,minieusopass -e "
       << "\"set ftp:passive-mode off;mrm *;quit\""
       << " 192.168.7.10" << "> /dev/null 2>&1" << std::endl;
      
  /* convert stringstream to char * */
  ftp_clear_str = conv1.str();
  ftp_clear = ftp_cmd_str.c_str();

  output = CpuTools::CommandToStr(ftp_clear);
 
  /* start FTP polling */
  clog << "info: " << logstream::info << "starting FTP server polling" << std::endl;
  
  /* build the command */
  conv2 << "lftp -u minieusouser,minieusopass -e "
       << "\"set ftp:passive-mode off;mirror --parallel=1 --verbose --Remove-source-files --ignore-time . /home/minieusouser/DATA;quit\""
       << " 192.168.7.10" << "> /dev/null 2>&1" << std::endl;
      
  /* convert stringstream to char * */
  ftp_cmd_str = conv2.str();
  ftp_cmd = ftp_cmd_str.c_str();

  if (monitor) {
    std::unique_lock<std::mutex> lock(this->_m_ftp);
    /* enter data processing loop while instrument mode switching not requested */
    while(!this->_cv_ftp.wait_for(lock,
				  std::chrono::milliseconds(WAIT_PERIOD),
				  [this] { return this->_ftp; }) ) { 
      
      output = CpuTools::CommandToStr(ftp_cmd);
      sleep(2);

    }
  }
  else {

    output = CpuTools::CommandToStr(ftp_cmd);
    
  }
  
}


/**
 * Look for new files in the data directory and process them depending on file type
 * @param ConfigOut output of the configuration file parsing with ConfigManager
 * @param CmdLine output of command line options parsing with InputParser
 * @param main_thread p_thread ID of the main thread which calls the function, used to send signals
 * @param scurve if true, blocks frm readout and only watches for Scurve and HV files 
 * uses inotify to watch the FTP directory and stops when signalled by 
 * DataAcquisition::Notify()
 */
int DataAcquisition::ProcessIncomingData(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine, long unsigned int main_thread, bool scurve) {
#ifndef __APPLE__
  int N_events, event_number = 0;
  int fd, wd;
  char buffer[BUF_LEN];

  std::string zynq_file_name;
  std::string sc_file_name;
  std::string hv_file_name;
  std::string data_str(DATA_DIR);
  std::string event_name;

  clog << "info: " << logstream::info << "starting background process of processing incoming data" << std::endl;

  /* initialise the inotify service */
  fd = inotify_init();
  if (fd < 0) {
    clog << "error: " << logstream::error << "unable to start inotify service" << std::endl;
  }

  /* watch the data directory for incoming files */
  clog << "info: " << logstream::info << "start watching " << DONE_DIR << std::endl;
  wd = inotify_add_watch(fd, DATA_DIR, IN_CLOSE_WRITE);

  /* to keep track of good and bad packets */
  int packet_counter = 0;
  int bad_packet_counter = 0;

  /* initilaise timeout timer */
  time_t start = time(0);
  int time_left = FTP_TIMEOUT;
  bool first_loop = true;
  
  std::unique_lock<std::mutex> lock(this->_m_switch);
  /* enter data processing loop while instrument mode switching not requested */
  while(!this->_cv_switch.wait_for(lock,
				       std::chrono::milliseconds(WAIT_PERIOD),
				   [this] { return this->_switch; }) /* no signal */
	&& (time_left > 0 || !first_loop) ) { /* no timeout */

    /* timeout if no activity after FTP_TIMEOUT reached */
    time_t end = time(0);
    time_t time_taken = end - start;
    time_left = FTP_TIMEOUT - time_taken;
    
    /* read out a set of inotify events into a buffer */
    struct inotify_event * event;
    N_events = read(fd, buffer, BUF_LEN);
    
    if (N_events < 0) {
      clog << "error: " << logstream::error << "unable to read from inotify file descriptor" << std::endl;
    }

    /* Loop through the events and read out the corresponding files */
    event_number = 0;
    while (event_number < N_events) {

      event = (struct inotify_event *) &buffer[event_number];
    
      if (event->len) {
	if (event->mask & IN_CLOSE_WRITE) {
	  
	  if (event->mask & IN_ISDIR) {
	  
	    /* process new directory creation */
	    printf("The directory %s was created\n", event->name);
	    clog << "info: " << logstream::info << "new directory created" << std::endl;
	  
	  }
	
	  else {
	  
	    /* process new file */
	    clog << "info: " << logstream::info << "new file created with name " << event->name << std::endl;
	    event_name = event->name;

	    
	    /* for files from Zynq (frm_cc_XXXXXXXX.data) */
	    if ( (event_name.compare(0, 3, "frm") == 0)
		 && (event_name.compare(event_name.length() - 3, event_name.length(), "dat") == 0) ) {

	      /* ignore frm files if waiting for an Scurve */
	      if(!scurve) {
		
		zynq_file_name = data_str + "/" + event->name;
	    
		/* new run file every RUN_SIZE packets */
		if (packet_counter == RUN_SIZE) {
		  CloseCpuRun(CPU);
	      
		  /* reset the packet counter */
		  packet_counter = 0;
		  std::cout << "PACKET COUNTER is reset to 0" << std::endl;
		  
		}
	    
		/* first packet */
		if (packet_counter == 0) {
	      
		  /* create a new run */
		  CreateCpuRun(CPU, ConfigOut, CmdLine);

		  /* reset first_loop status */
		  if (first_loop) {
		    first_loop = false;
		  }
		  
		}
	    	    
		/* generate sub packets */
		ZYNQ_PACKET * zynq_packet = ZynqPktReadOut(zynq_file_name, ConfigOut);
		HK_PACKET * hk_packet = AnalogPktReadOut();
		
		/* check for NULL packets */
		if ((zynq_packet != nullptr) && (hk_packet != nullptr)) {
	      
		  /* generate cpu packet and append to file */
		  WriteCpuPkt(zynq_packet, hk_packet, ConfigOut);
	      
		  /* delete upon completion */
		  if (!CmdLine->keep_zynq_pkt) {
		    std::remove(zynq_file_name.c_str());
		  }
	      
		  /* print update to screen */
		  printf("PACKET COUNTER = %i\n", packet_counter);
		  printf("The packet %s was read out\n", zynq_file_name.c_str());
	      
		  /* increment the packet counter */
		  packet_counter++;
	      
		/* leave loop for a single run file */
		if (packet_counter == CmdLine->acq_len-1 && CmdLine->single_run) {

		  /* send shutdown signal to RunInstrument */
		  /* interrupt signal to main thread */
		  pthread_kill((pthread_t)main_thread, SIGINT);
		  break;

		}
	      }
	    
		/* if NULL packets */
		else {

		  /* skip this packet */
		  bad_packet_counter++;

		}

	      }
	      else{
		
		/* do nothing if scurve = true */  

	      }
	    
	    } /* end of FRM packets */
	    
	  
	  
	    /* S-curve packets */
	    else if ( (event_name.compare(0, 2, "sc") == 0) &&
		      (event_name.compare(event_name.length() - 3, event_name.length(), "dat") == 0) ) {
	    
	      /* avoid timeout */
	      if (first_loop) {
		first_loop = false;
	      }
	    
	      sc_file_name = data_str + "/" + event->name;

	      /* wait for scurve completion */
	      std::unique_lock<std::mutex> sc_lock(this->_m_scurve);
	      while(!this->_cv_scurve.wait_for(sc_lock,
					       std::chrono::milliseconds(WAIT_PERIOD),
					       [this] { return this->_scurve; })) {}	
	      sleep(1);
	      std::cout << "S-curve acquisition complete" << std::endl;
	      
	      CreateCpuRun(SC, ConfigOut, CmdLine);
	      
	      /* generate sc packet and append to file */
	      SC_PACKET * sc_packet = ScPktReadOut(sc_file_name, ConfigOut);

	      if (sc_packet != NULL) {
		WriteScPkt(sc_packet);
	      }

	      /* print update to screen */
	      printf("The scurve %s was read out\n", sc_file_name.c_str());
	    
	      CloseCpuRun(SC);

	      /* delete upon completion */
	      if (!CmdLine->keep_zynq_pkt) {
		std::remove(sc_file_name.c_str());
	      }
              
	      /* exit without waiting for more files */
	      /* send shutdown signal to RunInstrument */
	      /* interrupt signal to main thread */
	      pthread_kill((pthread_t)main_thread, SIGINT);   
	      return 0;
	    
	    } /* end of SC packets */ 
	  
	  
	    /* for HV files from Zynq (hv_XXXXXXXX.dat) */
	    else if ( (event_name.compare(0, 2, "hv") == 0)
		      && (event_name.compare(event_name.length() - 3, event_name.length(), "dat") == 0) ) {
	      
	      /* avoid timeout */
	      if (first_loop) {

		/* create a new run */
		CreateCpuRun(CPU, ConfigOut, CmdLine);

		/* reset first_loop status */
		first_loop = false;
		
	      }
	    
	      hv_file_name = data_str + "/" + event->name;

	      /* Change so that you just pop HV packet inside existing CPU file  */
	      /* For now, do nothing to test */
	      //CreateCpuRun(HV, ConfigOut, CmdLine);
	    
	      /* generate hv packet to append to the file */
	      HV_PACKET * hv_packet = HvPktReadOut(hv_file_name, ConfigOut);
	      WriteHvPkt(hv_packet, ConfigOut);
	    
	      //CloseCpuRun(HV);
	    
	      /* delete upon completion */
	      std::remove(hv_file_name.c_str());

	      /* print update to screen */
	      //printf("PACKET COUNTER = %i\n", packet_counter);
	      printf("The HV packet %s was read out\n", hv_file_name.c_str());
	      
	    } /* end of HV packets */

	    
	    /* packet doesn't match any description */
	    else {
	    
	      /* do nothing and exit */
	      return 0;
	    
	    } /* end no matching packets */
	  
	  } /* if a file */
	} /* if event mode is CREATE */ 
      } /* if event->len */

      /* move to the next event in buffer */
      event_number += EVENT_SIZE + event->len;
      
    } /* loop of events in buffer */
      
  } /* end of while loop */

  /* stop watching the directory */
  inotify_rm_watch(fd, wd);
  close(fd);
#endif /* #ifndef __APPLE__ */
  return 0;
}

/**
 * read out the hv file into a HV_PACKET and store
 * @param ConfigOut output of the configuration file parsing with ConfigManager
 * called after the Zynq acquisition is stopped in 
 * DataAcquisition::CollectData()
 */
int DataAcquisition::GetHvInfo(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {

  std::string data_str(DATA_DIR);

  std::cout << "waiting for HV file... (NB: soon to be removed with FW update)" << std::endl;

  /* Poll the FTP server */
  FtpPoll(false);
  sleep(HV_FILE_TIMEOUT);
  
  /* get the filename */
  DIR * dir;
  struct dirent * ent;
  if ((dir = opendir(data_str.c_str())) != NULL) {

    /* check all files within directory */
    while ((ent = readdir(dir)) != NULL) {

      std::string fname(ent->d_name);
     
      if (fname.compare(0, 2, "HV") == 0) {

	/* just delete, HV file moved to main readout */
	/* read out the HV file, if it exists */
	std::string hv_file_name = data_str + "/" + fname;
	
        std::remove(hv_file_name.c_str());
  
	/* print update */
	clog << "info: " << logstream::info << "removed the the HVPS log file" << std::endl;
	std::cout << "removed the HVPS log file" << std::endl;
	
      }
      else {
	clog << "info: " << logstream::info << "no HV file found" << std::endl;
      }
    }
    closedir (dir);
  }
  else {
    clog << "error: " << logstream::error << "cannot open the data directory" << std::endl;
  }
  
  return 0;
}

/**
 * signal that the scurve is done to data gathering thread
 */
void DataAcquisition::SignalScurveDone() {
	
  {
    std::unique_lock<std::mutex> lock(this->_m_scurve);   
    this->_scurve = true;
  } /* release mutex */
  this->_cv_scurve.notify_all();
	
  return;
}
	
/**
 * check the scurve status in a thread-safe way
 */
bool DataAcquisition::IsScurveDone() {
  
  bool scurve_status;
  {
    std::unique_lock<std::mutex> lock(this->_m_scurve);   
    scurve_status = this->_scurve;
  }
	
  return scurve_status;
}

/**
 * spawn thread to collect an S-curve 
 * @param Zynq object to control the Zynq subsystem passed from RunInstrument
 * @param ConfigOut output of the configuration file parsing with ConfigManager
 * @param CmdLine output of command line options parsing with InputParser
 */
int DataAcquisition::CollectSc(ZynqManager * Zynq, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {
#ifndef __APPLE__

  long unsigned int main_thread = pthread_self();

  /* tell zynq to gather Scurve */
  {
    std::unique_lock<std::mutex> lock(Zynq->m_zynq);  
    Zynq->Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);
  }
  
  /* FTP polling */
  std::thread ftp_poll (&DataAcquisition::FtpPoll, this, false);
  
  /* collect the data */
  std::thread collect_data (&DataAcquisition::ProcessIncomingData, this, ConfigOut, CmdLine, main_thread, true);

  /* signal that scurve is done */
  this->SignalScurveDone();
  
  /* join threads */
  collect_data.join();
  ftp_poll.join();
  
#endif /* __APPLE__ */
  return 0;
}

/**
 * spawn threads to collect data 
 * @param Zynq object to control the Zynq subsystem passed from RunInstrument
 * @param ConfigOut output of the configuration file parsing with ConfigManager
 * @param CmdLine output of command line options parsing with InputParser
 * launches the required acquisition of different subsystems in parallel
 */
int DataAcquisition::CollectData(ZynqManager * Zynq, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine) {
#ifndef __APPLE__

  long unsigned int main_thread = pthread_self();

 #if ARDUINO_DEBUG !=1 
  /* FTP polling */
  std::thread ftp_poll (&DataAcquisition::FtpPoll, this, true);
  
  /* collect the data */
  std::thread collect_main_data (&DataAcquisition::ProcessIncomingData, this, ConfigOut, CmdLine, main_thread, false);
  
  /* set Zynq operational mode */
  /* select number of N1 and N2 packets */
  {
    std::unique_lock<std::mutex> lock(Zynq->m_zynq);  
    Zynq->SetNPkts(ConfigOut->N1, ConfigOut->N2);
    Zynq->SetL2TrigParams(ConfigOut->L2_N_BG, ConfigOut->L2_LOW_THRESH);
  }
  
  if (CmdLine->test_zynq_on) {
    /* set a mode to produce test data */
    {
      std::unique_lock<std::mutex> lock(Zynq->m_zynq);  
      Zynq->SetTestMode();   
    }
  }

  /* set a mode to start data gathering */
  {
    std::unique_lock<std::mutex> lock(Zynq->m_zynq);  
    Zynq->SetZynqMode();
  }
#endif

  
  /* add acquisition with the analog board */
  std::thread analog(&ArduinoManager::ProcessAnalogData, this->Analog, ConfigOut);
  
  /* wait for other acquisition threads to join */
  analog.join();

#if ARDUINO_DEBUG !=1
  collect_main_data.join();
  ftp_poll.join();
  
  /* only reached for instrument mode change */

  /* close the CPU file */
  CloseCpuRun(CPU);

  /* stop Zynq acquisition */
  {
    std::unique_lock<std::mutex> lock(Zynq->m_zynq);  
    Zynq->StopAcquisition();
  }
  
  /* read out HV file */
  GetHvInfo(ConfigOut, CmdLine);

#endif
  
#endif /* __APPLE__ */
  return 0;
}


/** 
 * function to generate and write a fake Zynq packet 
 * used for testing data format updates.
 * now the Zynq test acquisition modes can be used instead
 */
int DataAcquisition::WriteFakeZynqPkt() {

  ZYNQ_PACKET * zynq_packet = new ZYNQ_PACKET();
  Z_DATA_TYPE_SCI_L1_V2 * zynq_d1_packet_holder = new Z_DATA_TYPE_SCI_L1_V2();
  Z_DATA_TYPE_SCI_L2_V2 * zynq_d2_packet_holder = new Z_DATA_TYPE_SCI_L2_V2();
  std::shared_ptr<Config> ConfigOut = std::make_shared<Config>();
  
  uint32_t i, j, k = 0;

  /* set Config to dummy values */
  ConfigOut->N1 = 4;
  ConfigOut->N2 = 4;
  
  /* set data to dummy values */
  /* N packets */
  zynq_packet->N1 = ConfigOut->N1;
  zynq_packet->N2 = ConfigOut->N2;
  /* D1 */
  zynq_d1_packet_holder->payload.ts.n_gtu = 1;
  zynq_d1_packet_holder->payload.trig_type = 2;
  for (i = 0; i < 12; i++) {
    zynq_d1_packet_holder->payload.cathode_status[i] = 3;
  }
  for (i = 0; i < N_OF_FRAMES_L1_V0; i++) {
    for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
      zynq_d1_packet_holder->payload.raw_data[i][j] = 5;
    }
  }
  for (k = 0; k < zynq_packet->N1; k++) {
    zynq_packet->level1_data.push_back(*zynq_d1_packet_holder);
  }
  delete zynq_d1_packet_holder;
  /* D2 */
  zynq_d2_packet_holder->payload.ts.n_gtu = 1;
  zynq_d2_packet_holder->payload.trig_type = 2;
  for (i = 0; i < 12; i++) {
    zynq_d2_packet_holder->payload.cathode_status[i] = 3;
  }
  for (i = 0; i < N_OF_FRAMES_L2_V0; i++) {
    for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
      zynq_d2_packet_holder->payload.int16_data[i][j] = 5;
    }
  }
  for (k = 0; k < zynq_packet->N2; k++) {
    zynq_packet->level2_data.push_back(*zynq_d2_packet_holder);
  }
  delete zynq_d2_packet_holder;
  /* D3 */
  zynq_packet->level3_data.payload.ts.n_gtu = 1;
  zynq_packet->level3_data.payload.trig_type = 2;
  for (i = 0; i < 12; i++) {
    zynq_packet->level3_data.payload.cathode_status[i] = 3;
  } 
  for (i = 0; i < N_OF_FRAMES_L3_V0; i++) {
    for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
      zynq_packet->level3_data.payload.int32_data[i][j] = 5;
    }
  }
  
  /* open a SynchronisedFile */
  std::shared_ptr<SynchronisedFile> TestFile;
  Access * TestAccess;
 
  TestFile = std::make_shared<SynchronisedFile>("test_zynq_packet.dat");
  TestAccess = new Access(TestFile);
  
  /* write to file "test_zynq_file.dat" in current directory */
  //TestAccess->WriteToSynchFile<uint8_t *>(&zynq_packet->N1,
  //					  SynchronisedFile::CONSTANT);
  //TestAccess->WriteToSynchFile<uint8_t *>(&zynq_packet->N2,
  //					  SynchronisedFile::CONSTANT);
  TestAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L1_V2 *>(&zynq_packet->level1_data[0],
							SynchronisedFile::VARIABLE_D1, ConfigOut);
  TestAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L2_V2 *>(&zynq_packet->level2_data[0],
							SynchronisedFile::VARIABLE_D2, ConfigOut);
  TestAccess->WriteToSynchFile<Z_DATA_TYPE_SCI_L3_V2 *>(&zynq_packet->level3_data,
							SynchronisedFile::CONSTANT);
  TestAccess->CloseSynchFile();
  delete zynq_packet;  
  return 0;
} 

/** 
 * function to read a fake Zynq packet 
 * used for testing data format updates.
 * now the Zynq test acquisition modes can be used instead
 */
int DataAcquisition::ReadFakeZynqPkt() {

  FILE * fake_zynq_pkt = fopen("test_zynq_packet.dat", "rb");
  ZYNQ_PACKET * zynq_packet = new ZYNQ_PACKET;
  uint32_t i, j, k = 0;

  std::cout << "Starting to read file" << std::endl;
  
  /* get the size of vectors */
  //fread(&zynq_packet->N1, sizeof(zynq_packet->N1), 1, fake_zynq_pkt);
  //fread(&zynq_packet->N2, sizeof(zynq_packet->N2), 1, fake_zynq_pkt);
  zynq_packet->N1 = 4;
  zynq_packet->N2 = 4;
  
  /* resize the vectors */
  zynq_packet->level1_data.resize(zynq_packet->N1);
  zynq_packet->level2_data.resize(zynq_packet->N2);
  
  /* read in the vectors */
  for (i = 0; i < zynq_packet->level1_data.size(); i++) {
    fread(&zynq_packet->level1_data[i], sizeof(Z_DATA_TYPE_SCI_L1_V2), 1, fake_zynq_pkt);
  }
  for (i = 0; i < zynq_packet->level2_data.size(); i++) {
    fread(&zynq_packet->level2_data[i], sizeof(Z_DATA_TYPE_SCI_L2_V2), 1, fake_zynq_pkt);
  }
  fread(&zynq_packet->level3_data, sizeof(Z_DATA_TYPE_SCI_L3_V2), 1, fake_zynq_pkt);
  
  /* check against expected values */
  if (zynq_packet->N1 != 4) {
    std::cout << "error N1! N1 = " << zynq_packet->N1 << std::endl;
  }
  if (zynq_packet->N2 != 4) {
    std::cout << "error N2! N2 = " << zynq_packet->N2 << std::endl;
  }
  /* D1 */
  for (k = 0; k < zynq_packet->N1; k++) {
    for (i = 0; i < N_OF_FRAMES_L1_V0; i++) {
      for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
	if (zynq_packet->level1_data[k].payload.raw_data[i][j] != 5) {
	  std::cout << "error D1 data: "<< zynq_packet->level1_data[k].payload.raw_data[i][j] << std::endl;
	}
      }
    }
  }
  /* D2 */
  for (k = 0; k < zynq_packet->N2; k++) {
    for (i = 0; i < N_OF_FRAMES_L2_V0; i++) {
      for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
	if (zynq_packet->level2_data[k].payload.int16_data[i][j] != 5) {
	  std::cout << "error D2 data: "<< zynq_packet->level2_data[k].payload.int16_data[i][j] << std::endl;
	}
      }
    }
  }
  
  /* D3 */
  for (i = 0; i < N_OF_FRAMES_L3_V0; i++) {
    for (j = 0; j < N_OF_PIXEL_PER_PDM; j++) {
      if (zynq_packet->level3_data.payload.int32_data[i][j] != 5) {
	std::cout << "error D3 data: "<< zynq_packet->level3_data.payload.int32_data[i][j] << std::endl;
      }
    }
  }
    
    std::cout << "Read test complete! All OK unless error message shows" << std::endl;
    
    
    delete zynq_packet;
    fclose(fake_zynq_pkt);
    return 0;
}

