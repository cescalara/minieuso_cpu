Analog board
============

Description
-----------

The :cpp:class:`AnalogManager` handles the acquisition of data from the photodiodes, SiPMs and thermistors. Data is acquired using :cpp:func:`AnalogManager::GetLightLevel()` member function. This function uses the :cpp:func:`AnalogManager::SerialReadOut()` to handle the analog inputs on a separate board that is directly connected to the CPU board. This function is called in a separate joinable thread via the AnalogManager by both OperationMode daughter classes when they are operating to update the measurements periodically. Functions like :cpp:func:`RunInstrument::PollLightLevel()` make use of the :cpp:func:`AnalogManager::ReadLightLevel()` to read out the current stored LightLevel in the AnalogManager object in a thread safe way. The :cpp:func:`AnalogManager::CompareLightLevel()` handles the comparison of the average photodiode reading with set thresholds in order to set the mode to day/night. These thresholds are stored in ``AnalogManager.h`` as ``LIGHT_THRESHOLD``s and must be updated as the instrument calibration is carried out. The pinout of the analog board connected is described in the hardware interfaces section of the documentation. 

Reading photodiodes/SiPMs
-------------------------

The analog channels are read out via a serial interface to the analog board. A stream of data is sent constantly, so the software looks for packets by identifying the header tag. The details of the packet structure are still under development but will be updated here once settled. 

The analog channels are always read out, it is not necessary to add a flag to the main ``mecontrol`` executable. If no sensors are connected, the values on all channels should be ``-5``. If sensors are connected, some positive value should be read out. The exact value and its units will depend on the eventual calibration and light level.

Connection of the photodiodes/SiPMs/Thermistors
-----------------------------------------------

Please refer to the `hardware interfaces/analog_ports <http://minieuso-software.readthedocs.io/en/latest/hardware/analog_ports.html>`_ section for the connector pinout. 


AnalogManager
-------------

.. doxygenclass:: AnalogManager
   :path: ../CPU/CPUsoftware/doxygen/xml
   :members:
   :private-members:
