#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

//#include <iostream>
//#include <fstream>
#include <thread>

#include "log.h"

#define MIN_DEVICE_NUM 5 /* number of devices without extra storage or config USBs */
#define NOMINAL_DEVICE_NUM 9 /* number of devices expected */

#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

/* configuration for spare CPU in Stockholm */
#define STORAGE_BUS 2
#define CAMERA_BUS 1
#define STORAGE_BUS_0 1
#define STORAGE_BUS_1 2
#define STORAGE_PORT_0 2
#define STORAGE_PORT_1 1
#define CONFIG_BUS 0
#define CONFIG_PORT 0

class UsbManager {
public:
  uint8_t num_storage_dev;
  
  UsbManager();
  static int CheckUsb();
  uint8_t LookupUsbStorage();
  int RunDataBackup();
  int KillDataBackup();
  
private:
  std::thread::native_handle_type backup_thread_handle;  

  static void PrintDev(libusb_device * dev);
  int DataBackup();
};
#endif
/* _USB_INTERFACE_H */
