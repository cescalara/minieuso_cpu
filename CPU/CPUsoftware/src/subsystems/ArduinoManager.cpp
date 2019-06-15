#include "ArduinoManager.h"

/**
 * constructor 
 */
ArduinoManager::ArduinoManager() {

  this->light_level = std::make_shared<LightLevel>();
  this->analog_acq = std::make_shared<AnalogAcq>();
  int i = 0, j = 0;

  for (i = 0; i < FIFO_DEPTH; i++) {
    for (j = 0; j < CHANNELS; j++) {
      this->analog_acq->val[i][j] = 0;
    }
  }
  this->inst_mode_switch = false;

}


/**
 * analog board read out
 * uses the Arduino to collect data on the analog ports, 
 * as defined in ArduinoManager.h
 */
int ArduinoManager::AnalogDataCollect() {
#if ARDUINO_DEBUG ==1

  SerialReadOut(0x00);

#elif ARDUINO_DEBUG ==2

  int i, j;
  
  /* Just fill AnalogAcq with zeros */
  for (i = 0; i < FIFO_DEPTH; i++) {
    for (j = 0; j < CHANNELS; j++) {
      this->analog_acq->val[i][j] = 0;      
    }
  } 
  
#else
	/* test implementation for now, just prints output to screen */
	int fd;

	fd = open(DUINO, O_RDWR | O_NOCTTY | O_SYNC);

	if (fd < 0) {
		printf("Error opening %s: %s\n", DUINO, std::strerror(errno));
		return -1;
	}
	else {
		printf("Device has been opened and ready for operation! \n");
	}

	/*baudrate 9600, 8 bits, no parity, 1 stop bit */
	SetInterfaceAttribs(fd, BAUDRATE);
	printf("Will now run ArduinoManager::SerialReadOut() once...\n");

	SerialReadOut(fd);

#endif
  return 0;
}

/**
 * Read serial output from a file descriptor.
 */
// returns 0 if failed
int ArduinoManager::SerialReadOut(int fd) {

	unsigned char a[] = { 0xAA, 0x55, 0xAA, 0x55 };
	unsigned char buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER*4)];
	unsigned char temp_buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER * 4)];
#if ARDUINO_DEBUG ==1
	unsigned char simulated_buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER * 4)];
#endif
	unsigned int temp_checksum = 0;
	unsigned int buffer_checksum = 0;
	int checksum_passed = -1;

	std::string needle(a, a + 4);
  
	//	char buf[40*BUF_SIZE];
	unsigned int total_lenght = 0;
	unsigned   int len=50;
	//   char * p;
	//   char * err;
	unsigned  int i;
	unsigned   int ijk;
	//   double val;
	unsigned int start_search = 0; // ofsset to look of 0xaa55aa55
	unsigned int header_not_found = 0;
	
#if ARDUINO_DEBUG ==1
	simulated_buf[0] = 0xAA;
	simulated_buf[1] = 0x55;
	simulated_buf[2] = 0xAA;
	simulated_buf[3] = 0x55;
	for (i = 4; i < sizeof(simulated_buf); i++)
	  {
	    simulated_buf[i] = i;
	  }
	simulated_buf[10] = 0xAA;
	simulated_buf[11] = 0x55;
	simulated_buf[12] = 0xAA;
	simulated_buf[13] = 0x55;
	
 
	// calculate checksum
	temp_checksum = 0;
	for (ijk = 0; ijk < (X_TOTAL_BUF_SIZE / 2); ijk++)
	  {
	    temp_checksum += (simulated_buf[16 + ijk * 2 ] << 8) + simulated_buf[17 + ijk * 2 ];
	  }
	temp_checksum = temp_checksum & 0xFFFF;
	simulated_buf[16 + X_TOTAL_BUF_SIZE] = (temp_checksum >> 8) & 0xFF;
	simulated_buf[17 + X_TOTAL_BUF_SIZE] = (temp_checksum) & 0xFF;
#ifdef PRINT_DEBUG_INFO
	printf ("temp_checksum in fake buffer %x %x ", temp_checksum, simulated_buf[16 + X_TOTAL_BUF_SIZE]);
#endif
	temp_checksum = 0;
#endif
	
	/* repeat read to get full message */
	//for (i = 0; i < FIFO_DEPTH + 1; i++) 
	
	unsigned int Time_Elapsed = 0; // should be in ms, now is in attempts
	
	/* repeat until full data has arrived. at least two buffer size */
	/* should get time to put timeout */
	unsigned int MAX_Lenght = (X_TOTAL_BUF_SIZE_HEADER+ X_TOTAL_BUF_SIZE_HEADER);
#ifdef PRINT_DEBUG_INFO
	printf("\n sizeof(temp_buf) %d, sizeof(buf) %d \n", int(sizeof(temp_buf)), int(sizeof(buf)));
#endif
	while ((total_lenght < MAX_Lenght) && (Time_Elapsed < READ_ARDUINO_TIMEOUT) )
	  {
	    // clean temp_buf
	    for (ijk = 0; ijk < sizeof(temp_buf); ijk++)
	      {
		temp_buf[ijk] = 0;
	      }
	    
	    /* get number of bytes read */
#ifdef PRINT_DEBUG_INFO
	    printf("X_TOTAL_BUF_SIZE_HEADER %d (2 * X_TOTAL_BUF_SIZE_HEADER)  %d Time_Elapsed %d len %d total_lenght %d ", X_TOTAL_BUF_SIZE_HEADER, (MAX_Lenght), Time_Elapsed,len, total_lenght);
#endif
	    
#if ARDUINO_DEBUG ==1
	    for (ijk = 0; ijk < 50; ijk++)
	      {
		temp_buf[ijk] = simulated_buf[ijk+ total_lenght];
	      }
	    len = 50;
#else
	    len = read(fd, &temp_buf, sizeof(temp_buf)); // -1);
#endif
	    Time_Elapsed++;
	    for (ijk = 0; ijk<len; ijk++)
	      {
		buf[ijk + total_lenght] = temp_buf[ijk];
	      }
     total_lenght += len;
     
	  }
#ifdef PRINT_DEBUG_INFO
	printf("totallenght %d lenght %d", total_lenght, len);
#endif
	
	if (total_lenght < 0)
	  {
	    printf("Error from read: %d: %s\n", len, std::strerror(errno));
     return(0);
	  }
	else
   {
#ifdef PRINT_DEBUG_INFO
     /* print the serial output (debug) */
     printf("\n Begin Arduino Data Dump\n");
     
     for (ijk = 0; ijk < total_lenght; ijk++)
       {
	 printf(" %02x ", buf[ijk]);
       }
     
#endif
     
     /* get number of bytes read */
     //  len = read(fd, &buf, sizeof(buf)); // -1);
     //buf[BUF_SIZE-1] = '\0';
     start_search = 0;
     do
       {
	 
	 /* some bytes read */
	 //len = 250;
	 if (total_lenght > 0)
	   {
	     
	     // Look for AA55AA55
	     std::string haystack(buf, buf + sizeof(buf));  // or "+ sizeof Buffer"
	     std::size_t n = haystack.find(needle, start_search);
	     
	     if (n == std::string::npos)
	       {
#ifdef PRINT_DEBUG_INFO
		 printf("\n HEADER NOT FOUND");
#endif
		 header_not_found = 1; 
	       }
	     else
	       {
		 
		 //		  std::cout << "Position is  = " << n << std::endl;
#ifdef PRINT_DEBUG_INFO
		 printf("Position is %d ", (unsigned int)n); // position is n
#endif
		 this->analog_acq->val[0][0] = (buf[n + 6] << 8) + buf[n + 7];
		 this->analog_acq->val[0][1] = (buf[n + 8] << 8) + buf[n + 9];
		 this->analog_acq->val[0][2] = (buf[n + 10] << 8) + buf[n + 11];
		 this->analog_acq->val[0][3] = (buf[n + 12] << 8) + buf[n + 13];

#ifdef PRINT_DEBUG_INFO
		 printf(" packet number %d", (buf[n + 4] << 8) + buf[n + 5]);
		 printf(" zero %d", this->analog_acq->val[0][0]);
		 printf(" uno %d", this->analog_acq->val[0][1]);
		 printf(" due %d", this->analog_acq->val[0][2]);
		 printf(" tre %d", this->analog_acq->val[0][3]);
#endif
		 //this->analog_acq->val[0][0]=rand() % 150;
		 //printf("\n SerialReadout: randomizing %d", this->analog_acq->val[0][0]);
		 for (ijk = 0; ijk < X_SIPM_BUF_SIZE; ijk++)
		   {
		     this->analog_acq->val[0][ijk + 4] = (buf[n + 14 + ijk] << 8) + buf[n + 15 + ijk];
		   }
		 // calculate checksum
		 buffer_checksum = (buf[(n+X_TOTAL_BUF_SIZE + 6)] << 8) + buf[(n + X_TOTAL_BUF_SIZE + 7)];
		 temp_checksum = 0;
		 for (ijk = 0; ijk < (X_TOTAL_BUF_SIZE / 2); ijk++)
		   {
		     temp_checksum += (buf[n + ijk * 2 + 6] << 8) + buf[n + ijk * 2 + 6 + 1];
		   }
		 temp_checksum = temp_checksum & 0xFFFF;
		 if (temp_checksum == buffer_checksum)
		   {
#ifdef PRINT_DEBUG_INFO
		     printf("\n  checksum passed calc %x buffer %x stat_search %d \n ", temp_checksum, buffer_checksum, start_search);
#endif
		     checksum_passed = 1;
		   }
		 else
		   {
#ifdef PRINT_DEBUG_INFO
		     printf("\n  checksum FAILED calc %x buffer %x stat_search %d \n ", temp_checksum, buffer_checksum, start_search);
#endif
		     checksum_passed = 0;
		     start_search = n + 4;
#ifdef PRINT_DEBUG_INFO
		     printf("incremented star search %d", start_search);
#endif
		   }
	       }
	   }
	 
       } while (((checksum_passed == 0) && ((start_search + X_TOTAL_BUF_SIZE_HEADER) < total_lenght)) && (header_not_found==0));
   }
 if (checksum_passed == 1) return (1);
 else return (0); 
 
}

/**
 * get the current light level. 
 * preforms an analog acquisition using ArduinoManager::AnalogDataCollect()
 * and converts the output to the easily readable LightLevel format
 */
int ArduinoManager::GetLightLevel(std::shared_ptr<Config> ConfigOut) 
{
  int i, k;
  float ph[N_CHANNELS_PHOTODIODE];
  float sipm[N_CHANNELS_SIPM];
 
 
  /* interpret the analog acquisition struct */
  
  /* initialise */
  for(k = 0; k < N_CHANNELS_PHOTODIODE; k++) {
    ph[k] = 0;
  }
  for (k = 0; k < N_CHANNELS_SIPM; k++) {
    sipm[k] = 0;
  }
  

  /* read out the data */
  AnalogDataCollect();
  ph[0] = (float)(this->analog_acq->val[0][0]); // fixed at 0
  ph[1] = (float)(this->analog_acq->val[0][1]);
  ph[2] = (float)(this->analog_acq->val[0][2]);
  ph[3] = (float)(this->analog_acq->val[0][3]);
  
  for (k = 0; k < N_CHANNELS_SIPM; k++) {
    sipm[k] = (float)(this->analog_acq->val[0][4+k]);
  }
  
  /* DEBUG */
  std::cout << "PH 0:" << std::endl;
  std::cout << "val[0][0]" << this->analog_acq->val[0][0] << std::endl;
  std::cout << "ph[0]" << ph[0] << std::endl;
  std::cout << std::endl;
  
  std::cout << "PH 1:" << std::endl;
  std::cout << "val[0][1]" << this->analog_acq->val[0][1] << std::endl;
  std::cout << "ph[1]" << ph[1] << std::endl;
  std::cout << std::endl;

  std::cout << "PH 2:" << std::endl;
  std::cout << "val[0][2]" << this->analog_acq->val[0][2] << std::endl;
  std::cout << "ph[2]" << ph[2] << std::endl;
  std::cout << std::endl;
  
  std::cout << "PH 3:" << std::endl;
  std::cout << "val[0][3]" << this->analog_acq->val[0][3] << std::endl;
  std::cout << "ph[3]" << ph[3] << std::endl;
  std::cout << std::endl;

  std::cout << "SIPM 64 channels:" << std::endl;
  for (k=0; k < N_CHANNELS_SIPM; k++) {
    std::cout << this->analog_acq->val[0][k+4];
    std::cout << sipm[k]; 
  }
  std::cout << std::endl;

  
    /* read out the multiplexed 64 channel SiPM values */
   // {
   //std::unique_lock<std::mutex> lock(this->m_light_level);
   //   this->light_level->sipm_data[i] = this->analog_acq->val[i][5];
   // } /* release mutex */
  //}

  /* average the photodiode values */
  for (k = 0; k < N_CHANNELS_PHOTODIODE; k++) 
    {
      {
	std::unique_lock<std::mutex> lock(this->m_light_level);
	this->light_level->photodiode_data[k] = ph[k] / ConfigOut->average_depth;
      } /* release mutex */
      
    }
  //printf("\n GetLightLevel: photodiode_data: %f", this->light_level->photodiode_data[0]);
  /* average the one channel SiPM values */
  for (k = 0; k < N_CHANNELS_SIPM; k++)
    {
      {
	std::unique_lock<std::mutex> lock(this->m_light_level);
	this->light_level->sipm_data[k] = sipm[k] / ConfigOut->average_depth;
      } /* release mutex */
    }
  return 0;
}

/* 
 * read the light_level from object in a thread-safe way, 
 * without making an acquisition 
 */
std::shared_ptr<LightLevel> ArduinoManager::ReadLightLevel() {
  
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
 
  return light_level; 
}


/**
 * compare light level to threshold value
 * LIGHT_THRESHOLD from configuration file config/dummy.conf or config/dummy_local.conf
 * @TODO check if more sophisticated tests needed in lab
 */
ArduinoManager::LightLevelStatus ArduinoManager::CompareLightLevel(std::shared_ptr<Config> ConfigOut) {
  
  LightLevelStatus current_lightlevel_status = LIGHT_UNDEF;
  float ph_avg = 0;
  int i;
  
#if ARDUINO_DEBUG != 1 
  clog << "info: " << logstream::info << "comparing light level to day and night thresholds" << std::endl;
#else
  printf("comparing light level to day and night thresholds \n"); 
#endif   

  //  printf("CompareLightLevel: light level before GetLightLevel %f \n", this->light_level->photodiode_data[0]);
  this->GetLightLevel(ConfigOut);
  //printf("CompareLightLevel: light level after GetLightLevel %f \n", this->light_level->photodiode_data[0]);
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
  
  /* read the light level */
  /* average the 4 photodiode values */
  // {
  //   std::unique_lock<std::mutex> lock(this->m_light_level);
  //   for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
  //     ph_avg += light_level->photodiode_data[i];
  //   }
  // } /* release mutex */
  // ph_avg = ph_avg / (float)N_CHANNELS_PHOTODIODE;

  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    ph_avg = light_level->photodiode_data[ConfigOut->ana_sensor_num];
  }
  
  /* debug */
 #if ARDUINO_DEBUG != 1
  clog << "info: " << logstream::info << "average photodiode reading is: " << ph_avg << std::endl;
#else
  printf("light photodiode reading is: %f \n", light_level->photodiode_data[ConfigOut->ana_sensor_num]);
#endif
  
  /* compare the result to day and night thresholds */
  if (ph_avg >= ConfigOut->day_light_threshold) {
    //printf("\n CompareLightLevel: photodiode_data: %f , %d", this->light_level->photodiode_data[0], ConfigOut->day_light_threshold);
    current_lightlevel_status = ArduinoManager::LIGHT_ABOVE_DAY_THR;
#if ARDUINO_DEBUG != 1
    clog << "info: " << logstream::info << "light level is ABOVE day_light_threshold" << std::endl;
#else		
    printf("light level is ABOVE day_light_threshold \n"); 
#endif		
  }
  
  else if (ph_avg <= ConfigOut->night_light_threshold) {
    current_lightlevel_status = ArduinoManager::LIGHT_BELOW_NIGHT_THR;
#if ARDUINO_DEBUG != 1
    clog << "info: " << logstream::info << "light level is BELOW night_light_threshold" << std::endl;
#else		
    printf("light level is BELOW night_light_threshold \n");
#endif	
  }
  
  else if (ph_avg > ConfigOut->night_light_threshold && ph_avg < ConfigOut->day_light_threshold) {
    current_lightlevel_status = ArduinoManager::LIGHT_UNDEF;
#if ARDUINO_DEBUG != 1
    clog << "info: " << logstream::info << "light level is BETWEEN  night_light_threshold and day_light_threshold" << std::endl;
#else		
    printf("light level is BETWEEN night_light_threshold and day_light_threshold \n");
#endif	 
  }
  
  /* set the config to allow info to be passed around */
  // this->ConfigOut->lightlevel_status = current_lightlevel_status;
  
  return current_lightlevel_status;
}


int ArduinoManager::ProcessAnalogData(std::shared_ptr<Config> ConfigOut) {

  std::unique_lock<std::mutex> lock(this->m_mode_switch);
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(ConfigOut->arduino_wait_period),
				       [this] { return this->inst_mode_switch; })) { 
    this->GetLightLevel(ConfigOut);
    //#if ARDUINO_DEBUG == 0
    sleep(ConfigOut->light_acq_time);
    //#endif
  }
  return 0;
}


/**
 * reset the mode switching after an instrument mode change
 * used by OperationMode::Reset() 
 */
int ArduinoManager::Reset() {
  
  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = false;
  } /* release mutex */

  /* update measurement */
  //  this->GetLightLevel(ConfigOut);

  return 0;
}

/**
 * notify the object of an instrument mode switch 
 * used by OperationMode::Notify
 */
int ArduinoManager::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = true;
  } /* release mutex */
  
  return 0;
}

/**
 * Set up interface attributes for the interface with the Arduino device.
 */
int ArduinoManager::SetInterfaceAttribs(int fd, int speed) {
#if ARDUINO_DEBUG ==0
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    printf("Error from tcgetattr: %s\n", std::strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);
  tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;         /* 8-bit characters */
  tty.c_cflag &= ~PARENB;     /* no parity bit */
  tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 5;
  
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error from tcsetattr: %s\n", std::strerror(errno));
    return -1;
  }
#endif  
  return 0;
}












