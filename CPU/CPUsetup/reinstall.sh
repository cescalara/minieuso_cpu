#!/bin/bash

# CPU REINSTALL SCRIPT for Mini-EUSO
# Francesca Capel February 2018
# capel.francesca@gmail.com
# Mini-EUSO software repository: https://github.com/cescalara/minieuso_cpu
# Please see documentation for correct setup procedure

HOME_DIR="/home/software/CPU"

echo "************************"
echo "Mini-EUSO CPU reinstall"
echo "***********************"

# Set up the Mini-EUSO software
echo "Setting up the Mini-EUSO software..."
mkdir $HOME_DIR/CPUsoftware/log > /dev/null 2>&1
(cd $HOME_DIR/CPUsoftware/lib && make)
(cd $HOME_DIR/CPUsoftware && make)
echo "Mini-EUSO software is set up"

# Set up the cameras 
echo "Setting up the camera software..."
#sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
mkdir $HOME_DIR/cameras/multiplecam/bin > /dev/null 2>&1
chmod +x $HOME_DIR/cameras/multiplecam/src/install.sh
(cd $HOME_DIR/cameras/multiplecam/src && sh -c './install.sh /home/software/CPU/cameras/multiplecam')
echo "Camera software is set up"

