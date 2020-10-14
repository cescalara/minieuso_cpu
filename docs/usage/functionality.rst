Functionality
=============

General
-------

* ``mecontrol -help`` show a list of all the command line options
* ``mecontrol -ver`` show the version info
* ``mecontrol -db`` run a debug program for diagnostic tests of all subsystems

**NB: the use of command line arguments will always override the corresponding values specified in the configuration file**. This is beacsue command line use is intended for testing in the lab, whereas the configuration file specifies these values more robustly for automated use on the ISS. 
  

Control of the high voltage (HV)
--------------------------------

The CPU controls the HV sent to the PMTs via the Zynq board. 

**Important: always ensure that the instrument is in a dark, light-tight environment and that all LEDs on the instrument itself are covered or disabled before switching on the HV. Similarly, ensure the HV is switched off before allowing light on the instrument.**

* The voltage is automatically ramped up in steps of ~ 140 V from 0 V to the desired operating voltage
* The HV can be turned on in 2 different ways, with or without an automated acquisition
  
  * ``mecontrol -hvswitch on -hv all -dv <X>`` switches on the HV for all ECs then exits the program, to allow for other tests
  * ``mecontrol -hvswitch on -hv 0,0,0,0,0,0,0,0,1 -dv <X>`` switches on the HV for onlt one EC unit then exits the program, to allow for other tests
  * ``mecontrol -hv all -dv <X>`` starts an acquisition, turning the HV on and off automatically for all EC units 
  * ``mecontrol -hv 0,0,0,0,0,0,0,0,1 -dv <X>`` starts an acquisition, turning the HV on and off automatically for only one EC unit
  * ``mecontrol -hv 0,0,0,0,0,0,0,0,1 -dv <X,X,X,X,X,X,X,X,X>`` can also be used to set individual dynode voltage values for each EC unit
  
* To switch off the HV, simply use

  * ``mecontrol -hvswitch off`` switches off the HV for all EC units
    
* Explanation of the command flags

  * the flag ``-dv <X>`` sets the dynode voltage to X, where X is the DAC between 0 and 4096 
  * the flag ``-dvr <X>`` sets the dynode voltage to X, where X is the voltage in VOLTS between 0 and 1100
  * the conversion between this number ``<X>`` and a voltage is ``<X>/4096 * 2.44 * 466``, ie. ``DAC/maxDAC * maxDAC_voltage * 466``
  * the flag ``-asicdac <Y>`` sets the ASIC dac level, or the point in an S-curve that we want to use to acquire data (should be in the S-curve plateau), this will override the ``DAC_LEVEL`` set in the configuration file
  * the ``-asicdac`` flag was previously ``-hvdac`` but this was changed to avoid confusion, however, for the sake of compatibility, ``-hvdac`` can still be used with identical functionality
  * if these flags are not supplied, their default values are used from the configuration file in ``CPUsoftware/config``

* To check the current status, use ``mecontrol -check_status``
* If an acquisition with HV is interrupted using ``CTRL-C``, the HV will be switched off automatically

  
Control of the low voltage power supply (LVPS)
----------------------------------------------

The CPU controls the powering of the other sub-systems via the LVPS.

* The main data acquisition program will automatically turn on/off subsystems as they are needed
* There are also commands to switch on and off subsystems, then exit the program

  * ``mecontrol -lvps on -subsystem <S>`` will switch on ``<S>`` which can be ``zynq``, ``cam`` or ``hk``
  * ``mecontrol -lvps off -subsystem <S>`` will switch off the specified sub-system

The Zynq power is left on by default when running subsequent ``mecontrol`` commands. If you want to reboot the zynq upon starting a new acquisition, use the following:

* ``mecontrol -zynq_reboot`` 


    
Data acquisition
----------------

The CPU handles the data acquisition from all subsystems. 

* Summary of main command options

  * ``-scurve``: take a single S-curve and exit, to overwrite the configuration file options use:

    * ``-start``: start ASIC DAC for threshold scan (min = 0)
    * ``-step``: step between consecutive ASIC DAC acquisitions
    * ``-stop``: stop ASIC DAC for threshold scan (max = 1023)
    * ``-acc``: number of GTU taken at each ASIC DAC step
      
  * ``-short <N>``: run a short acquisition of ``<N>`` CPU_PACKETs (NB: ``<N>`` must be less than ``RUN_SIZE`` defined in minieuso_data_format.h)
  * ``-zynq <MODE>``: use the Zynq acquisition mode (see section below for details, default = ``periodic``)
  * ``-test_zynq <MODE>``: use the Zynq test mode (see section below for details, default = ``pdm``)
  * ``-keep_zynq_pkt``: keep the Zynq packets on FTP
  * ``-comment`` : add a string comment which is put in the :cpp:class:`CpuFileHeader` and the CPU file name (e.g. ``-comment "your comment here"``).
    
* An example use case: ``mecontrol -log -test_zynq pdm -keep_zynq_pkt`` would start and acquisition in Zynq pdm test mode and keep the Zynq packets on the FTP server to check them

* Data from the PDM is collected as specified by the command line options, packets are sent from the Zynq every 5.24s with 3 levels of data (D1, D2 and D3) and information on timestamping and the HV status. 

  * The number of packets for D1 and D2 data, N1 and N2, can be set in the configuration file
  * Analog and housekeeping data are also gathered every 5.24 s and packaged together with the Zynq data into a CPU packet
  * One CPU packet is appended to the current CPU run file every 5.24s
  * Data from the cameras is collected independently with both cameras operating simultaneously

* if no USB storage is available, the output data from the CPU is in ``/home/minieusouser/DONE`` with filenames of the form ``CPU_RUN_<run_type>__<current_date>__<current_time>.dat``

  * S-curve files also have the configured dynode voltage appended to the filename, even if the HV is not switched on 
  * the data format of these files is documented in ``CPUsoftware/src/data_format/data_format.h`` 
  * log files are in ``CPUsoftware/log``, if log output is switched on with ``-log``

* the output data from the cameras is in ``cameras/multiplecam/<NIR/VIS>/<current_date>``

  * .raw for the photos from the cameras
  * log files are in ``cameras/multiplecam/log/``

* if USB storage is detected on the system, the output files will be automatically written there instead, and backed up if there is more than one device.

   
Zynq acquisition modes
----------------------

The Zynq handles the collection of data from the PMTs via the SPACIROC3 ASICs. There are many different ways to collect this data, which are described here.

**Main acquisiton modes**

There are five basic acquisition modes, as defined here (for ``TA_TRIGGER`` see EUSO-TA trigger section below):

.. doxygenenum:: ZynqMode
		 

In this section, the term "trigger" is used to describe an event which leads to a data collection of one packet from D1, D2 and D3 (ie. 128 GTU of the first level data, 128 GTU of the second level data and 128 GTU of the thrid level data). N1 and N2 are the number of packets of D1 and D2 data required, and are defined in the configuration file. One data cycle refers to every 5.24 s.

* ``none``: no acquistion, setting this mode can also used to stop and existing acquisition
* ``periodic``: the Zynq uses an internal pulse generator to acquire N1 D1 packets and N2 D2 packets every data cycle, there is always only 1 D3 packet per cycle   
* ``self``: the Zynq uses the built in L1 and L2 trigger algorithms, a *maximum* of N1 D1 packets and N2 D2 packets are stored every data cycle
* ``immediate``: a single trigger is collected in a controlled way for debugging purposes via a COM-port keypress or by sending the telnet command ``trg``
* ``external``: a single trigger is collected in a controlled way via an external electrical pulse

To set the desired mode, use the flag ``-zynq <MODE>`` with the ``mecontrol`` command. Any combination of the above modes can be set simultaneously, simply separate them with a ``,``. For example, the following command::

  mecontrol -zynq periodic,self

Will start an acquisition using both ``periodic`` and ``self`` modes. This means data will be acquired using the built in L1 and L2 trigger algorithms, and if no trigger is detected, the Zynq will simply collect data using it's internal pulse generator. This is the standard operational mode of the instrument and thus can also be accessed with the following command, for simplicity::

  mecontrol -zynq trigger

The multi-level trigger is described in detail in A. Belov et al., *The integration and testing of the Mini-EUSO multi-level trigger system*. Advances in Space Reasearch (2017).

**EUSO-TA trigger mode**

* ``ta_trigger``: This sets the EUSO-TA trigger mode for testing. At the moment, it is not possible to use this in combination with any of the other trigger modes described above.

**Test acquisition modes**

The Zynq also has built in test modes for debugging, where data is provided by the software instead of  collected from the ASICs. These modes are defined in ZynqManager::TestMode and are descibed here. 

.. doxygenenum:: TestMode

* ``none``: normal operation, data provider test generator is switched OFF.
* ``ecasic``: all pixels are 0 in EC ESIC board #0, all pixels =  6  in EC ESIC board #1, all pixels =  12 in EC ESIC board #2, all pixels =  18 in EC ESIC board #3, all pixels =  24 in EC ESIC board #4, all pixels =  30 in EC ESIC board #5, all frames are the same
* ``pmt``: all pixels = 0 in PMT #0, all pixels = 1 in PMT #1, ..., all pixels = 35 in PMT #35, all frames are the same
* ``pdm``:  all pixels = 0 in 1st frame, all pixels = 1 in 2nd frame, ..., all pixels = 127 in 128th frame, after 128 frames counter resets to 0
* ``l1``: all pixels = 0 in 1st 128 frames, all pixels = 1 in 2nd 128 frames, ..., after 128*128 frames counter resets to 0
* ``l2``: all pixels = 0 in 1st 128*128 frames, all pixels = 1 in 2nd 128*128 frames, ..., after 128*128*128 frames counter resets to 0
* ``l3``: all pixels = 0 in 1st 128*128*128 frames, all pixels = 1 in 2nd 128*128*128 frames, ..., after 128*128*128*128 frames counter resets to 0

To set the desired mode, use the flag ``-test_zynq <MODE>`` with the ``mecontrol`` command. The test modes can only be used one at a time.


The configuration file
----------------------

The configuration file stores the following parameters:

* ``CATHODE_VOLTAGE``: the cathode voltage to set the HV to via the Zynq command ``hvps cathode`` (can be 0,1,2 or 3 and the default is 3 (fully switched on)) 
* ``DYNODE_VOLTAGE``: the dynode voltage to set the HV to via the Zynq command ``hvps setdac`` (can be 0 to 4096), it is be overidden by the command line option ``-dv``
* ``SCURVE_START``: the ASIC DAC at which to start scanning the thresholds for an S-curve (default is 0)
* ``SCURVE_STEP``: the ASIC DAC steps to take between consecutive S-curve acquisitions (default is 8)
* ``SCURVE_STOP``: the maximum ASIC DAC to scan to when taking an S-curve (default and maximum is 1023)
* ``SCURVE_ACC``: the number of acquisitions to take at a certain ASIC DAC during an S-curve (default is 16384)
* ``DAC_LEVEL``: the ASIC DAC level at which to perform standard acquisitions (non S-curve) (default is 500)
* ``N1``: maximum number of packets to be stored for D1, the level 1 data (can be 1 to 4, default is 4)
* ``N2``: maximum number of packets to be stored for D1, the level 1 data (can be 1 to 4, default is 4)

The default values are stored in the file ``config/dummy.conf``. To override these values without recompiling the software edit ``config/dummy_local.conf``, or for certain fields (HV and S-curve parameters) use the command line options described above. Both methods work, so whatever is most convenient.

When the software is launched into an acquisition mode, the final configuration used in the program is printed to the screen with the title "Configuration Parameters".

