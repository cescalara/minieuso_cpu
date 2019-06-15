#ifndef _OPERATION_MODE_H
#define _OPERATION_MODE_H

#include <mutex>
#include <condition_variable>

#include "AnalogManager.h"
#include "ThermManager.h"
#include "ConfigManager.h"

/** 
 * base class for an operational mode 
 */
class OperationMode {
public:
  /**
   * control of the analog subsystem 
   */
  AnalogManager * Analog = new AnalogManager();
  /**
   * control of the thermal subsystem
   */
  ThermManager * Thermistors = new ThermManager();

  std::shared_ptr<Config> ConfigOut;

  OperationMode();
  
  /* operation control functions */
  virtual void Start();
  void Stop();
  
  void Notify();
  void Reset();

protected:
  /**
   * to handle mode switching in a thread-safe way 
   */
  std::mutex _m_switch;
  /**
   * to wait for a mode switch 
   */
  std::condition_variable _cv_switch;
  /**
   * to notify of a mode switch 
   */
  bool _switch;

  /**
   * to handle swicthing in a thread safe way 
   */
  std::mutex _m_ftp;
  /**
   * to wait for an ftp switch 
   */
  std::condition_variable _cv_ftp;
  /**
   * to notify of a mode switch 
   */
  bool _ftp;

};


#endif
/* _OPERATION_MODE_H */
