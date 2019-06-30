#ifndef _RUN_INSTRUMENT_H
#define _RUN_INSTRUMENT_H

#include "LvpsManager.h"
#include "UsbManager.h"
#include "CamManager.h"
#include "DataAcquisition.h"
#include "DataReduction.h"
#include "AnalogManager.h"
#include "ConfigManager.h"

/* location of data files */
#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

/* signal to not set the dac_level */
#define NO_DAC_SET -99

/* wait time between status checks in seconds */
//#define STATUS_PERIOD 30

/* wait between switching on HV and Zynq in seconds */
//#define PWR_ON_DELAY 2

/**
 * class to handle different instrument operational modes
 */
class RunInstrument {
public:
  CmdLineInputs * CmdLine;
  /**
   * enum to specify the current operational mode of the instrument
   */
  enum InstrumentMode : uint8_t {
    NIGHT = 0,
    DAY = 1,
    INST_UNDEF = 2,
  };
  InstrumentMode current_inst_mode;
  std::mutex m_inst_mode;

  /**
   * enum to specify the current acquisition mode of the instrument
   */
  enum AcquisitionMode : uint8_t {
    STANDARD = 0,
    SCURVE = 1,
    ACQ_UNDEF = 2,
  };
  AcquisitionMode current_acq_mode;

  std::shared_ptr<Config> ConfigOut;
  ZynqManager Zynq;
  LvpsManager Lvps;
  UsbManager Usb;
  CamManager Cam;
  DataAcquisition Daq;
  DataReduction Data;

  AnalogManager::LightLevelStatus current_lightlevel_status;

  RunInstrument(CmdLineInputs * CmdLine);
  void Start();
  void Stop();

  int SetInstMode(InstrumentMode mode_to_set);
  InstrumentMode GetInstMode();

private:
  /**
  * to write is_day.txt
  * Giammanco 05/2019
  **/
  std::ofstream isDay;

  /**
   * to handle stopping
   */
  std::mutex _m_stop;
  std::condition_variable _cv_stop;
  bool _stop;

  /**
   * start-up procedure
   */
  int StartUp();

  /**
   * execute-and-exit commands
   */
  int LvpsSwitch();
  int HvpsSwitch();
  int DebugMode();
  int CheckStatus();

  /**
   * initialisation
   */
  int InitInstMode();
  int CheckSystems();
  int SelectAcqOption();

  /**
   * used in operations
   */
  static void SignalHandler(int signum);
  int LaunchCam();
  int Acquisition();
  int MonitorInstrument();
  int PollInstrument();
  int StatusChecker();
  int RunningStatusCheck();
  int SetStop();
  bool CheckStop();

  /**
   * define main operational procedures
   */
  int NightOperations();
  int DayOperations();
};

#endif
/* _RUN_INSTRUMENT_H */
