#!/bin/bash

platform=$(uname -s)

if [ "$platform" == "Darwin" ]
then
    ln -sf $(PWD)/bin/mecontrol /usr/local/bin/mecontrol
else
    LinuxOS=$(lsb_release -si)

    if [ "$LinuxOS" != "Ubuntu" ]
    then
	ln -sf /home/software/CPU/CPUsoftware/bin/mecontrol /usr/local/bin/mecontrol
    fi
fi

