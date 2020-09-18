Configuration file
==================

In order to allow for flexible use during flight, a number of parameters can be set in the configuration file, some examples of which are stored `here <https://github.com/cescalara/minieuso_cpu/tree/master/CPU/CPUsoftware/config>`_. The software looks for files on an external USB pen, but if it is not found a local file is used with the nominal settings. Once found, the file is parsed using :cpp:class:`ConfigManager` and the parameters are passed around the software using the :cpp:class:`Config` data structure. Key parameters are also strored in the :cpp:class:`CpuFileHeader` for easy reference.

Configurable parameters
-----------------------

The following parameters can be set in the configuration file:

* ``CATHODE_VOLTAGE``: The level at which to set the cathode voltage (0 <=> off, 1 <=> same as last dynode, 3 <=> full) 
* ``DYNODE_VOLTAGE``: The DAC voltage level at which to set the dynode voltage (an integer between 0 and 4095)  
* ``SCURVE_START``: The DAC pulse threshold at which to start S-curve acquisition (an integer between 0 and 1023) 
* ``SCURVE_STEP``: The DAC step to take in sweeping the pulse threshold in an S-curve (an integer between 0 and 1023)
* ``SCURVE_STOP``: The DAC pulse threshold at which to stop S-curve acquisition (an integer between 0 and 1023)
* ``SCURVE_ACC``: The number of frames to average over during each step of the S-curve acquisition (an integer between 1 aand 999999)
* ``DAC_LEVEL``: The DAC pulse threshold to use during standard data acquisition - usually set by looking at S-curves (an integer between 0 and 1023)
* ``N1``: Number of level 1 (D1) data packets to be read out by the Zynq (an integer between 1 and 8)
* ``N2``: Number of level 2 (D2) data packets to be read out by the Zynq (an integer between 1 and 8)
* ``L2_N_BG``: Level 2 trigger - number of times above background level to set threshold (an integer between 2 and max value if unsigned int)
* ``L2_LOW_THRESH``: Level 2 trigger - lower bound to threshold considered (an integer between 0 and max value of unsigned int)
* ``ARDUINO_WAIT_PERIOD``: The wait period in *milliseconds* used to check between instrument mode switching (an integer between 1 and 1000)
* ``ANA_SENSOR_NUM``: Index of the analog sensor used to determine mode switching (an integer between 0 and 3)
* ``AVERAGE_DEPTH``: Depth of analog acquisitions to calculate an average over (an integer between 1 and 3)
* ``DAY_LIGHT_THRESHOLD``: Minimum light threshold for day mode 
* ``NIGHT_LIGHT_THRESHOLD``: Maximum light threshold for night mode 
* ``LIGHT_POLL_TIME``: Period in *seconds* for updating the light level and thermistor readings
* ``LIGHT_ACQ_TIME``: Period in *seconds* for reading out a :cpp:class:`THERM_PACKET` 
* ``STATUS_PERIOD``: Period in *seconds* for printing a general status check to the screen and logs
* ``PWR_ON_DELAY``: Delay in *seconds* between switching on the Zynq and the high voltage
* ``CAMERA_ON``: Select which camera to launch acquisition with (11 <=> both, 10 <=> NIR only, 01 <=> VIS only) 
