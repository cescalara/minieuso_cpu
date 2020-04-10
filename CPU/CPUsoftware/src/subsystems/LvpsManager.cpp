#include "LvpsManager.h"

/**
 * constructor.
 * sets all statuses to UNDEF
 */
LvpsManager::LvpsManager() {

  /* status initialisation */
  this->zynq_status = UNDEF;
  this->cam_status = UNDEF;
  this->hk_status = UNDEF;

  /* initialise ports */
  int i;
  for (i = 0; i < 4; i++) {
    this->P1Bits[i] = 0;
  }
  
}

/**
 * get the current status of a subsystem 
 */
LvpsManager::Status LvpsManager::GetStatus(SubSystem sub_system) {
  
  switch (sub_system) {
  case ZYNQ:
    return this->zynq_status;
    break;
  case CAMERAS:
    return this->cam_status;
    break;
  case HK:
    return this->hk_status;
    break;   
  }
  return UNDEF;
}

/**
 * switch on a subsystem and check its return line
 * @param subsytem the subsytem to be switched on 
 */
int LvpsManager::SwitchOn(SubSystem sub_system) {

  clog << "info: " << logstream::info << "switching on " << sub_system << std::endl;

  switch (sub_system) {
  case ZYNQ:
    SetPulseP0(ZYNQ_PORT_ON);
    this->zynq_status = ON;
    break;
  case CAMERAS:
    SetPulseP0(CAMERA_PORT_ON); 
    this->cam_status = ON;
    break;
  case HK:
    SetPulseP0(HK_PORT_ON);
    this->hk_status = ON;
    break;
  }

  /* check switched on */
  if (Check(sub_system)) {
    clog << "info: " << logstream::info << sub_system << " was switched on correctly" << std::endl;
  }
  else {
    clog << "error: " << logstream::error << sub_system << " was not switched on correctly" << std::endl;
  }
  
  return 0;
}

/**
 * switch off a subsystem and check its return line
 * @param subsystem the subsystem to be switched off
 */
int LvpsManager::SwitchOff(SubSystem sub_system) {
  
  clog << "info: " << logstream::info << "switching off " << sub_system << std::endl;
  
  switch (sub_system) {
  case ZYNQ:
    SetPulseP0(ZYNQ_PORT_OFF);
    this->zynq_status = OFF;
    break;
  case CAMERAS:
    SetPulseP0(CAMERA_PORT_OFF); 
    this->cam_status = OFF;
    break;
  case HK:
    SetPulseP0(HK_PORT_OFF);
    this->hk_status = OFF;
    break;
  }

  /* check switched off */
  if (!Check(sub_system)) {
    clog << "info: " << logstream::info << sub_system << " was swicthed off correctly" << std::endl;
  }
  else {
    clog << "info: " << logstream::info << sub_system << " was not switched off correctly" << std::endl;
  }
  
  return 0;
}

/**
 * check the return line.
 * returns true if on and false if off 
 * @param subsystem the subsystem to check
 */
bool LvpsManager::Check(SubSystem sub_system) {

  bool return_status = false;
  int hk_port_check = 0;
  int zynq_port_check = 1;
  int camera_port_check = 2;
  
  clog << "info: " << logstream::info << "checking status of " << sub_system << std::endl;

  /* read from P1 to write this->P1Bits */
  ReadP1();
  
  switch (sub_system) {
  case ZYNQ:
    if (this->P1Bits[zynq_port_check] == 1) {
      return_status = true;
    }
    break;
  case CAMERAS:
    if (this->P1Bits[camera_port_check] == 1) {
      return_status = true;
    }
    break;
  case HK:
    if (this->P1Bits[hk_port_check] == 1) {
      return_status = true;
    }
    break;
  }
  
  return return_status;
}

/**
 * initialise the aDIO ports 
 */
int LvpsManager::InitPorts() {
#ifndef __APPLE__
  int aDIO_ReturnVal;
 
  aDIO_ReturnVal = OpenDIO_aDIO(&aDIO_Device, minor_number);
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  OpenDIO_aDIO(%u) FAILED: MinorNumber(= %u) maybe incorrect",
	  minor_number, minor_number);
    clog << "error: " << logstream::error << "could not initialise CPU aDIO ports" << std::endl;
    return 1;
  }
  
  return 0;
#endif /* __APPLE__ */
  return 0;
}

/**
 * write the direction of Port1 
 */
int LvpsManager::SetDirP1(uint8_t port_config) {
#ifndef __APPLE__
  int aDIO_ReturnVal;

  /* write the direction of port 1 */
  aDIO_ReturnVal =
    LoadPort1PortDir_aDIO(aDIO_Device, port_config);

  /* sleep 1 ms */
  usleep(ONE_MILLISEC);
  
  /* check the return value */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  LoadPort1bitDir_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not set direction of CPU aDIO port 1 to " << port_config << std::endl;
    return 1;
  }

  return 0;
#endif /* __APPLE__ */
  return 0;
}


/**
 * read the values of Port1 
 */
int LvpsManager::ReadP1() {
#ifndef __APPLE__ 
  int aDIO_ReturnVal;
  int Bit = 0;
  uint8_t read_value;

  /* initialise */
  InitPorts();
  for (Bit = 0; Bit < 4; Bit++) {
    this->P1Bits[Bit] = 0;
  }
  
  /* set the P1 direction to input */
  SetDirP1(PORT1_INPUT);
  
  /* read the required port */
  aDIO_ReturnVal = ReadPort_aDIO(aDIO_Device, 1, &read_value);

  /* sleep 1 ms */
  usleep(ONE_MILLISEC);
  
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  ReadPort_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not read value from port 1" << std::endl;
  }

  /* separate out into bits */
  for (Bit = 0; Bit < 4; Bit++) {
    this->P1Bits[Bit] = (read_value >> Bit) & 0x01;
  }

  /* clean up and exit */
  CloseDev();
 
#endif /* __APPLE__ */
  return 0;
} 


/**
 * write the direction of Port0 
 * @param port_config hex value to allow bit-programmable direction setting, e.g. 0xFF = 1111 1111 (all Port0 set to output)
 *
 */
int LvpsManager::SetDirP0(uint8_t port_config) {
#ifndef __APPLE__
  int aDIO_ReturnVal;
  uint8_t P0Bits[8];
  int Bit = 0;

  /* write the direction of port 0 */
  for (Bit = 0; Bit < 8; Bit++) {
    P0Bits[Bit] = (port_config >> Bit) & 0x01;
  }
  
  /* set the bits of port 0 */
  aDIO_ReturnVal =
    LoadPort0BitDir_aDIO(aDIO_Device, P0Bits[7],
			 P0Bits[6], P0Bits[5],
			 P0Bits[4], P0Bits[3],
			 P0Bits[2], P0Bits[1],
			 P0Bits[0]);
  /* sleep 1 ms */
  usleep(ONE_MILLISEC);
  
  /* check the return value */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  LoadPort0bitDir_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not set direction of CPU aDIO port 0 to " << port_config << std::endl;
    return 1;
  }

  return 0;
#endif /* __APPLE__ */
  return 0;
}


/**
 * set the value of Port0 
 * @param port_value the value to set
 */
int LvpsManager::SetValP0(PortValue port_value) {
#ifndef __APPLE__
  int aDIO_ReturnVal = 0;
  
  switch (port_value) {
  case HIGH:
    aDIO_ReturnVal =
      WritePort_aDIO(aDIO_Device, 0, HIGH);
    break;
  case LOW:
    aDIO_ReturnVal =
      WritePort_aDIO(aDIO_Device, 0, LOW);
    break;
  }
 
  usleep(ONE_MILLISEC); 

  /* check the return */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  WritePort_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not set value of CPU aDIO P0 " << port_value << std::endl; 
    return 1;
  }

  return 0;
#endif /* __APPLE__ */
  return 0;
}

/**
 * close the aDIO device
 */
int LvpsManager::CloseDev() {
#ifndef __APPLE__
  int aDIO_ReturnVal;
  
  aDIO_ReturnVal = CloseDIO_aDIO(aDIO_Device);
  if (aDIO_ReturnVal) {
    clog << "error: " << logstream::error << "could not close CPU aDIO " << aDIO_ReturnVal << std::endl;
  }

  return 0;
#endif /*  __APPLE__ */
  return 0;
}

/**
 * deliver a 5V, 10 ms pulse to a certain pin or pins of Port0 
 * @param port_config specifies which pins eg. 0xFF = 1111 1111 (all pins pulsed)
 */
int LvpsManager::SetPulseP0(uint8_t port_config) {

  /* initialise */
  InitPorts();
  
  /* set specified output */
  SetDirP0(port_config);
 
  /* write high */
  SetValP0(HIGH);
   
  /* sleep for 9 ms */
  usleep(9 * ONE_MILLISEC);

  /* write low */
  SetValP0(LOW);

  /* clean up and exit */
  CloseDev();
    
  return 0;
}

