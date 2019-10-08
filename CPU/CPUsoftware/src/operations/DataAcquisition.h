#ifndef _DATA_ACQUISITION_H
#define _DATA_ACQUISITION_H

#ifndef __APPLE__
#include <sys/inotify.h>
#endif /* __APPLE__ */
#include <thread>

#include "OperationMode.h"
#include "ThermManager.h"
#include "AnalogManager.h"
#include "InputParser.h"
#include "ConfigManager.h"

#define DATA_DIR "/home/minieusouser/DATA"
#define DONE_DIR "/home/minieusouser/DONE"
#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

/* maximum filename size (CPU is Ext4 but USB is FAT32) */
#define MAX_FILENAME_LENGTH 255

/* for use with inotify in ProcessIncomingData() */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define FTP_TIMEOUT 10 /* seconds */

/* number of seconds to wait for HV file transfer on FTP */
#define HV_FILE_TIMEOUT 1


/** NIGHT operational mode: data acquisition
 * class for controlling the main acquisition 
 * (the Zynq board, the thermistors and the Analog board)
 */
class DataAcquisition : public OperationMode {   
public:  
  std::string cpu_main_file_name;
  std::string cpu_sc_file_name;
  std::string cpu_hv_file_name;
  uint8_t usb_num_storage_dev;
  int n_files_written;
  std::mutex m_nfiles;
  
  /**
   * synchronised file pointer
   */
  std::shared_ptr<SynchronisedFile> CpuFile;
  /**
   * synchronised file access
   */
  Access * RunAccess;
  /**
  * output of the configuration parsing is stored here
  */
  std::shared_ptr<Config> ConfigOut;


  /**
   * enum to define the CPU file type
   */
  enum RunType : uint8_t {
    CPU = 0,
    SC = 1,
    HV = 2,
  };

  DataAcquisition();
  int CreateCpuRun(RunType run_type, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  int CloseCpuRun(RunType run_type);
  int CollectSc(ZynqManager * ZqManager, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  int CollectData(ZynqManager * ZqManager, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  bool IsScurveDone();
  static int WriteFakeZynqPkt();
  static int ReadFakeZynqPkt();
  
private:
  /**
   * to handle scurve acquisition in a thread-safe way
   */
  std::mutex _m_scurve;
  /**
   * to wait for scurve acquisition to complete
   */
  std::condition_variable _cv_scurve;
  /**
   * to notify a completed scurve
   */
  bool _scurve;  

  std::string CreateCpuRunName(RunType run_type, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  std::string BuildCpuFileInfo(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  SC_PACKET * ScPktReadOut(std::string sc_file_name, std::shared_ptr<Config> ConfigOut);
  HV_PACKET * HvPktReadOut(std::string hv_file_name, std::shared_ptr<Config> ConfigOut);
  ZYNQ_PACKET * ZynqPktReadOut(std::string zynq_file_name, std::shared_ptr<Config> ConfigOut);
  HK_PACKET * AnalogPktReadOut();
  int WriteScPkt(SC_PACKET * sc_packet);
  int WriteHvPkt(HV_PACKET * hv_packet, std::shared_ptr<Config> ConfigOut);
  int WriteCpuPkt(ZYNQ_PACKET * zynq_packet, HK_PACKET * hk_packet, std::shared_ptr<Config> ConfigOut);
  int GetHvInfo(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  int GetScurve(ZynqManager * Zynq, std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine);
  void FtpPoll(bool monitor);
  int ProcessIncomingData(std::shared_ptr<Config> ConfigOut, CmdLineInputs * CmdLine, long unsigned int main_thread, bool scurve);
  void SignalScurveDone();
  
};

#endif
/* _DATA_ACQUISITION_H */
