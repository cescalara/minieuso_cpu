Mini-EUSO Software
===================
[![Build Status](https://travis-ci.org/cescalara/minieuso_cpu.svg?branch=master)](https://travis-ci.org/cescalara/minieuso_cpu) [![Documentation Status](https://readthedocs.org/projects/minieuso-software/badge/?version=latest)](http://minieuso-software.readthedocs.io/en/latest/?badge=latest)  [![Codacy Badge](https://api.codacy.com/project/badge/Grade/8a8b19d626334b75a5d5e88e89afcbca)](https://www.codacy.com/manual/cescalara/minieuso_cpu?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=cescalara/minieuso_cpu&amp;utm_campaign=Badge_Grade) [![DOI](https://zenodo.org/badge/72226620.svg)](https://zenodo.org/badge/latestdoi/72226620)  [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This is the repository for the Mini-EUSO instrument software. If you make use of this code in your work, please cite the above DOI and the paper listed below.

Documentation
-------------

The software documentation can be found here: http://minieuso-software.readthedocs.io  
*(powered by [Doxygen](http://www.stack.nl/~dimitri/doxygen/), [Sphinx](http://www.sphinx-doc.org/), [breathe](https://breathe.readthedocs.io) and [Read the Docs](https://readthedocs.org/))*

Further information on the status of the Mini-EUSO integration and testing can be found on the Mini-EUSO wiki page: https://jemeuso.riken.jp/wiki/index.php?Mini-EUSO


Paper
-----

A more detailed description of the software design can be found here: https://arxiv.org/abs/1907.04938.

Acknowledgements
----------------

I would like to acknowledge the following contributions to the software:
* S. Turriziani: `multiplecam` software for ancillary cameras
* L. Marcelli: improving day/night thresholds and configuration file
* G. Cambiè & M. Casolino: Final `AnalogManager::SerialReadOut()` and testing
* C. Giammanco: pixel masking and automated boot scripts
