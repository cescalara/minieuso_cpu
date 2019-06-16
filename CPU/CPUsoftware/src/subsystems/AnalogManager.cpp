#include "AnalogManager.h"

/**
 * constructor 
 */
AnalogManager::AnalogManager() {

  this->light_level = std::make_shared<LightLevel>();
  this->analog_acq = std::make_shared<AnalogAcq>();
  this->temperature_acq = std::make_shared<TemperatureAcq>();
  
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
 * as defined in AnalogManager.h
 */
int AnalogManager::AnalogDataCollect() {
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

  /* run the actual analog readout from the Arduino  */
  int fd;

  fd = open(DUINO, O_RDWR | O_NOCTTY | O_SYNC);

  if (fd < 0) {

    clog << "error: " << logstream::error << "error opening Arduino " << DUINO << " " << std::strerror(errno) << std::endl;
    return -1;
    
  }
  else {

    clog << "info: " << logstream::info << "Device " << DUINO << " has been opened and ready for operation" << std::endl;

  }

  /*baudrate 9600, 8 bits, no parity, 1 stop bit */
  SetInterfaceAttribs(fd, BAUDRATE);
  
  clog << "info: " << logstream::info << "Will now run AnalogManager::SerialReadOut()" << std::endl;
  SerialReadOut(fd);

#endif
  return 0;
}

/**
 * Read serial output from a file descriptor.
 */
// returns 0 if failed
int AnalogManager::SerialReadOut(int fd) {

	unsigned char a[] = { 0xAA, 0x55, 0xAA, 0x55 };
	unsigned char buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER*4)];
	unsigned char temp_buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER*4)];
#if ARDUINO_DEBUG ==1
	unsigned char simulated_buf[(unsigned int)(X_TOTAL_BUF_SIZE_HEADER*4)];
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
		     this->analog_acq->val[0][ijk + 4] = (buf[n + 14 + (2*ijk)] << 8) + buf[n + 15 + (2*ijk)];
		   }

		 for (ijk=0; ijk < N_CHANNELS_THERM; ijk++) {

		   float converted_temp_output = 0;
		   char raw_temp_output[9];
		   int ijkl;
		   
		   /* get 9 byte temp info */
		   for (ijkl = 0; ijkl < 9; ijkl++) {
		     raw_temp_output[ijkl] =  buf[(n + X_TOTAL_BUF_SIZE + 8 + (ijk*9) + ijkl)];
		   }
		   
		   /* convert to float */
		   converted_temp_output = ConvertToTemp(raw_temp_output);
		   
		   /* assign to analog_acq struct */
		   //this->analog_acq->val[0][N_CHANNELS_PHOTODIODE+N_CHANNELS_SIPM+ijk] = converted_temp_output; 
		   //std::cout << "Therm " << ijk << " :" << converted_temp_output << std::endl;
		   
		   /* debug */
		   /* For now, just assign known numbers while those in Rome continue the debugging */
		   this->analog_acq->val[0][N_CHANNELS_PHOTODIODE+N_CHANNELS_SIPM+ijk] = 26; 
		   
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
 * preforms an analog acquisition using AnalogManager::AnalogDataCollect()
 * and converts the output to the easily readable LightLevel format
 */
int AnalogManager::GetLightLevel(std::shared_ptr<Config> ConfigOut) 
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
  /*
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
    std::cout << this->analog_acq->val[0][k+4] << " ";
    std::cout << sipm[k] << " ";
    std::cout << "    ";
  }
  std::cout << std::endl;
  */
  
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
	this->light_level->photodiode_data[k] = ph[k];
      } /* release mutex */
      
    }
  //printf("\n GetLightLevel: photodiode_data: %f", this->light_level->photodiode_data[0]);
  /* average the one channel SiPM values */
  for (k = 0; k < N_CHANNELS_SIPM; k++)
    {
      {
	std::unique_lock<std::mutex> lock(this->m_light_level);
	this->light_level->sipm_data[k] = sipm[k];
      } /* release mutex */
    }

  /* read out the thermistors */
  for (k = 0; k < N_CHANNELS_THERM; k++)
    {
      {
	std::unique_lock<std::mutex> lock(this->m_temperature_acq);
	this->temperature_acq->val[k] = (float)this->analog_acq->val[0][N_CHANNELS_PHOTODIODE+N_CHANNELS_SIPM+k];
      }
    }
  
  return 0;
}

/* 
 * read the light_level from object in a thread-safe way, 
 * without making an acquisition 
 */
std::shared_ptr<LightLevel> AnalogManager::ReadLightLevel() {
  
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
AnalogManager::LightLevelStatus AnalogManager::CompareLightLevel(std::shared_ptr<Config> ConfigOut) {
  
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
    current_lightlevel_status = AnalogManager::LIGHT_ABOVE_DAY_THR;
#if ARDUINO_DEBUG != 1
    clog << "info: " << logstream::info << "light level is ABOVE day_light_threshold" << std::endl;
#else		
    printf("light level is ABOVE day_light_threshold \n"); 
#endif		
  }
  
  else if (ph_avg <= ConfigOut->night_light_threshold) {
    current_lightlevel_status = AnalogManager::LIGHT_BELOW_NIGHT_THR;
#if ARDUINO_DEBUG != 1
    clog << "info: " << logstream::info << "light level is BELOW night_light_threshold" << std::endl;
#else		
    printf("light level is BELOW night_light_threshold \n");
#endif	
  }
  
  else if (ph_avg > ConfigOut->night_light_threshold && ph_avg < ConfigOut->day_light_threshold) {
    current_lightlevel_status = AnalogManager::LIGHT_UNDEF;
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


int AnalogManager::ProcessAnalogData(std::shared_ptr<Config> ConfigOut) {

  std::mutex m;
  
  std::unique_lock<std::mutex> lock(this->m_mode_switch);
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(ConfigOut->arduino_wait_period),
				       [this] { return this->inst_mode_switch; })) { 

    /* get the light level and read out thermistors  */
    this->GetLightLevel(ConfigOut);

    /* write THERM_PACKET to file if night/data acquisition */
    if (this->current_lightlevel_status == LIGHT_BELOW_NIGHT_THR) { 
      /* from old ThermManager::ProcessThermData() */
      /* wait for CPU file to be set by DataAcqManager::ProcessIncomingData() */
      std::unique_lock<std::mutex> lock(m);
      this->cond_var.wait(lock, [this]{return cpu_file_is_set == true;});

      if (this->temperature_acq != NULL) {
	WriteThermPkt();
      }
    }
    
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
int AnalogManager::Reset() {
  
  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = false;
  } /* release mutex */

  /* update measurement */
  //this->GetLightLevel(ConfigOut);

  return 0;
}

/**
 * notify the object of an instrument mode switch 
 * used by OperationMode::Notify
 */
int AnalogManager::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = true;
  } /* release mutex */
  
  return 0;
}

/**
 * Set up interface attributes for the interface with the Arduino device.
 */
int AnalogManager::SetInterfaceAttribs(int fd, int speed) {
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

/**
 * Convert the data to actual temperature
 * because the result is a 16 bit signed integer, it should
 * be stored to an "int16_t" type, which is always 16 bits
 * even when compiled on a 32 bit processor.
 */
float AnalogManager::ConvertToTemp(char data[9]) {
  
 int16_t raw = (data[1] << 8) | data[0];
 char type_s = 1; /* to be fixed */
 float celsius;
 
 if (type_s) {

   raw = raw << 3; // 9 bit resolution default
   if (data[7] == 0x10) {
     // "count remain" gives full 12 bit resolution
     raw = (raw & 0xFFF0) + 12 - data[6];
   }

 }
 else {

   char cfg = (data[4] & 0x60);
   // at lower res, the low bits are undefined, so let's zero them
   if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
   else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
   else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
   //// default is 12 bit resolution, 750 ms conversion time

 }
 celsius = (float)raw / 16.0;

 return celsius;
}

/*
 * write the temperature packet to file 
 * @param temperature_results contains the parsed temperature data
 */
int AnalogManager::WriteThermPkt() {

  THERM_PACKET * therm_packet = new THERM_PACKET();
  static unsigned int pkt_counter = 0;
  
  clog << "info: " << logstream::info << "writing new therm packet to " << this->RunAccess->path << std::endl;
  /* create the therm packet header */
  therm_packet->therm_packet_header.header = CpuTools::BuildCpuHeader(THERM_PACKET_TYPE, THERM_PACKET_VER);
  therm_packet->therm_packet_header.pkt_size = sizeof(*therm_packet);
  therm_packet->therm_packet_header.pkt_num = pkt_counter; 
  therm_packet->therm_time.cpu_time_stamp = CpuTools::BuildCpuTimeStamp();

  if (this->temperature_acq != NULL) {
    /* get the temperature data */
    for (int i = 0; i < N_CHANNELS_THERM; i++) {
      therm_packet->therm_data[i] = this->temperature_acq->val[i];
    }
  }
  
  /* write the therm packet */
  this->RunAccess->WriteToSynchFile<THERM_PACKET *>(therm_packet, SynchronisedFile::CONSTANT);
  delete therm_packet; 
  pkt_counter++;

  return 0;
}










