#!/bin/bash

# CPU SETUP SCRIPT for Mini-EUSO
# Francesca Capel February 2018
# capel.francesca@gmail.com
# Mini-EUSO software repository: https://github.com/cescalara/minieuso_cpu
# Please see documentation for correct setup procedure

HOME_DIR="/home/software/CPU"
TEST_IP=8.8.8.8

echo "*******************"
echo "Mini-EUSO CPU setup"
echo "*******************"

# Turn off the annoying beeps
setterm -blength 0

# Set the timeout of raising the network interface
cp $HOME_DIR/CPUsetup/reduce_timeout.conf /lib/systemd/system/networking.service.d/

# Download the necessary packages
if ping -q -c 1 -W 1 $TEST_IP > /dev/null 2>&1
then
   echo "Connected to internet..."
   echo "Downloading packages..."
   apt-get -y update 
   apt-get -y install build-essential vsftpd expect libraw1394-11 libgtk2.0-0 \
   libgtkmm-2.4-dev libglademm-2.4-dev libgtkglextmm-x11-1.2-dev libusb-1.0-0 \
   libusb-1.0 stress bridge-utils git-core emacs usbmount gdb ntp libboost-all-dev \
   inotify-tools digitemp libasan lsb_release linux-headers-3.16.0-6-amd64 lftp
   echo "Packages downloaded"
   echo "*******************"
   echo "*******************"
else
       echo "Could not connect to internet. Exiting..."
       exit 1
fi

# Set up the FTP server and necessary directories
echo "Setting up the FTP server and directories..."
mkdir /home/minieusouser/DATA > /dev/null 2>&1
chown minieusouser /home/minieusouser/DATA
mkdir /home/minieusouser/DONE > /dev/null 2>&1
chown minieusouser /home/minieusouser/DONE 
rm /etc/vsftpd.conf > /dev/null 2>&1
cp $HOME_DIR/CPUsetup/vsftpd.conf /etc/ > /dev/null 2>&1
mkdir /media/usb > /dev/null 2>&1
mkdir /home/minieusouser/log  > /dev/null 2>&1
echo "FTP server is set up"
echo "*******************"
echo "*******************"

# Set up the telnet scripts
chmod +x $HOME_DIR/zynq/telnet/*

# Set up the Mini-EUSO software
echo "Setting up the Mini-EUSO software..."
mkdir $HOME_DIR/CPUsoftware/log > /dev/null 2>&1
(cd $HOME_DIR/CPUsoftware/lib && make)
(cd $HOME_DIR/CPUsoftware && make)
echo "Mini-EUSO software is set up"
echo "****************************"
echo "****************************"

# Setup symlinks for commands
echo "Creating symlinks"
# correct digitemp command
ln -s /usr/bin/digitemp_DS9097U /usr/local/bin/digitemp

# Network configuration 
echo "Setting up the network configuration..."
cp $HOME_DIR/CPUsetup/interfaces /etc/network/ > /dev/null 2>&1
echo "Network configuration is set up"
echo "*******************************"
echo "*******************************"

# Set up the cameras 
echo "Setting up the camera software..."
chmod +x $HOME_DIR/cameras/flycapture2-2.3.2.14-amd64/install_flycapture.sh
(cd $HOME_DIR/cameras/flycapture2-2.3.2.14-amd64 && sh install_flycapture.sh)
rm /etc/default/grub
cp $HOME_DIR/CPUsetup/grub /etc/default/
update-grub
#sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
mkdir $HOME_DIR/cameras/multiplecam/bin > /dev/null 2>\&1
chmod +x $HOME_DIR/cameras/multiplecam/src/install.sh
(cd $HOME_DIR/cameras/multiplecam/src && sh -c './install.sh $HOME_DIR/cameras/multiplecam')
echo "Camera software is set up"
echo "*************************"
echo "*************************"
 
# Set up the aDIO ports on CPU
echo "Setting up the aDIO port software..."
echo "rtd_aDIO" >> /etc/modules
make -C $HOME_DIR/aDIO/driver
(cd $HOME_DIR/aDIO/driver && make load)
cp $HOME_DIR/aDIO/driver/rtd-aDIO.ko /lib/modules/$(uname -r)/kernel/rtd/
(cd $HOME_DIR/aDIO/driver && depmod -a)
echo "lsmod | grep rtd:"
lsmod | grep rtd
echo "aDIO software is set up"
echo "***********************"
echo "***********************"

# Set the local time to UTC
timedatectl set-timezone UTC
hwclock --systohc

# Set up autologin to root 
echo "Setting up autologin to root user on boot..."
mkdir /etc/systemd/system/getty@tty1.service.d
touch /etc/systemd/system/getty@tty1.service.d/autologin.conf
> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "[Service]" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=-/sbin/agetty -a root --noclear %I $TERM" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "OK"
echo "********************"
echo "CPU setup completed!"
echo "********************"

# Press enter to continue
read -p "Press enter to continue, the system will reboot to make changes."

systemctl daemon-reload
# Restart the CPU
reboot









   
   
       
   


