.. Mini-EUSO instrument software documentation master file, created by
   sphinx-quickstart on Fri Feb  9 10:40:30 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Mini-EUSO software
==================

The Mini-EUSO instrument is designed by the JEM-EUSO collaboration to pave the way for space-based observations of Extreme Energy Cosmic Rays (EECRs). To be placed inside the International Space Station (ISS) in early 2018, it is a small UV (300 - 400 nm) telescope which will observe the Earth’s atmosphere with a spatial resolution of 6.11 km. Mini-EUSO is capable of detecting a wide variety of UV events such as cosmic ray signals, transient luminous events and meteors with a minimum time resolution of 2.5 μs.

The flight software is fully automated and takes advantage of the frequent day/night cycles of the ISS orbit and ancillary instruments with which Mini-EUSO is equipped in order to optimise the mission’s scientific output. The software is responsible for the control of all instrument subsystems and data acquisition. This is achieved with an object oriented design using C++11 and is documented here. For more information on the high-level functionality of the software, see F. Capel et al., *Mini-EUSO flight software and operations on ISS*, In: Proceedings of the 35th International Cosmic Ray Conference (2017). For a full description of the Mini-EUSO instrument and its scientific goals see F. Capel et al., *Mini-EUSO: A high resolution detector for the study of terrestrial and cosmic UV emission from the International Space Station*, Advances in Space Research (2017).

The source code for this project can be found on GitHub: https://github.com/cescalara/minieuso_cpu

Further information on the status of the Mini-EUSO integration and testing as well as technical documnments can be found on the Mini-EUSO wiki page: https://jemeuso.riken.jp/wiki/index.php?Mini-EUSO (members only).

Contents
========
.. toctree::
   :maxdepth: 2
	     
   installation
   update
   ssh_connection
   usage
   configuration_file
   automated_acquisition
   data_format
   data_visualisation
   test_procedure
   hardware_interfaces
   development
   issues
   source_code
