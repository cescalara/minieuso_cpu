#ifndef _ZYNQ_INTERFACE_H
#define _ZYNQ_INTERFACE_H

#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <fstream>
#include <algorithm>
#include <mutex>

#include "log.h"
#include "CpuTools.h"
/*Giammanco include the pxel mask*/
#include "DeadPixelRead.h"

/* interface to Zynq board */
#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23
#define CONNECT_TIMEOUT_SEC 100
#define SHORT_TIMEOUT_SEC 2

/* pedestal for the ASIC DAC */
#define PEDESTAL 750

/* for use with HV interface functions */
#define N_EC 9
#define N_PMT 36
#define N_ASIC 6

/* time between consecutive telnet commands in mus */
#define SLEEP_TIME 500000

/* location of Zynq setup files */
#define ZYNQ_SETUP_SUBDIR "/automated_boot"
#define MATRIX_DAC_10 "dac10.txt"


/**
 * class to handle the Zynq interface. 
 * commands and information are sent and received over telnet
 * using socket programming.
 * data from the Zynq board is placed on the FTP directory
 */
class ZynqManager {
public:

  /**
   * the zynq acquisition mode options
   */
  enum ZynqMode : uint8_t {
    /**
     * basic modes 
     */
    NONE = 0,
    PERIODIC = 2,
    SELF = 4,
    IMMEDIATE = 8,
    EXTERNAL = 16,
    TA_TRIGGER = 32, /* EUSO-TA TRIGGER, cannot be used in compound modes */
 
    /**
     * compound modes 
     */
    TRIGGER = 6, /* usual data taking mode: PERIODIC + SELF */
    PERIODIC_IMMEDIATE = 10, /* PERIODIC + IMMEDIATE */
    SELF_IMMEDIATE = 12, /* SELF + IMMEDIATE */
    PERIODIC_SELF_IMMEDIATE = 14, /* PERIODIC + SELF + IMMEDIATE  */
    PERIODIC_EXTERNAL = 18, /* PERIODIC + EXTERNAL */
    SELF_EXTERNAL = 20, /* SELF + EXTERNAL */
    PERIODIC_SELF_EXTERNAL = 22, /* PERIODIC + SELF + EXTERNAL */
    IMMEDIATE_EXTERNAL = 24, /* IMMEDIATE + EXTERNAL */ 
    PERIODIC_IMMEDIATE_EXTERNAL = 26, /* PERIODIC + IMMEDIATE + EXTERNAL */
    SELF_IMMEDIATE_EXTERNAL = 28, /* SELF + IMMEDIATE + EXTERNAL */
    ALL_TRIGGER = 30, /* PERIODIC + SELF + IMMEDIATE + EXTERNAL */
  };
  /**
   * stores the current zynq acquisiton mode
   */
  uint8_t zynq_mode;

  /**
   * the zynq test mode options
   */
  enum TestMode : uint8_t {
    T_NONE = 0,
    ECASIC = 1,
    PMT = 2,
    PDM = 3,
    L1 = 4,
    L2 = 5,
    L3 = 6,
  };
  /*
   * stores the current zynq test mode
   */
  TestMode test_mode;

  /**
   * the status options for the HV
   */
  enum HvpsStatus : uint8_t {
    OFF = 0,
    ON = 1,
    UNDEF = 2,
  };
  /**
   * stores the current HV status
   */
  HvpsStatus hvps_status;

  /**
   * set to true if the telnet connection is successful 
   */
  bool telnet_connected;
  /**
   * vector of EC values (0 <=> off, 1 <=> on)
   */
  std::vector<int> ec_values;

  /*
   * to handle Zynq access in a thread safe way
   */
  std::mutex m_zynq;
  
  ZynqManager();
  int CheckConnect();
  static int ConnectTelnet();
  int GetInstStatus();
  int GetHvpsStatus();
  int HvpsTurnOn(int cv, std::string hvps_dv_string, std::string hvps_ec_string);
  int HvpsTurnOff();
  int HidePixels(); /*added by Giammanco*/
  int Scurve(int start, int step, int stop, int acc);
  int SetDac(int dac_level);
  int AcqShot();
  uint8_t SetZynqMode();
  TestMode SetTestMode();
  static int StopAcquisition();
  int SetNPkts(int N1, int N2);
  int SetL2TrigParams(int n_bg, int low_thresh); 
  bool CheckScurve(int sockfd);
  static std::string GetZynqVer();
  int InstrumentClean();
  int Reboot();
  int SetMatrixDac10(std::string usb_mountpoint, bool debug);
  int Setup(std::string setup_script_path);
  
private:
  
  static std::string SendRecvTelnet(std::string send_msg, int sockfd);
  static int SendTelnet(std::string send_msg, int sockfd);
  static std::string Telnet(std::string send_msg, int sockfd, bool print);
  static int TelnetSendOnly(std::string send_msg, int sockfd); 
  int InstStatusTest(std::string send_msg);
  bool CheckTelnet();  

};

#endif /* _ZYNQ_INTERFACE_H */
