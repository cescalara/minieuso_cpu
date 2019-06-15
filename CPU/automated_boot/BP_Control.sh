##!/bin/bash
#
T=864000                 #set time for whole program duration in sec
t=864000                 #set time for single run duration in sec
S=1                      #set time between one check and another (smaller than t) in sec
cmd='./SimulaEuso.sh'    #string command to activate mecontrol
p_name='mecontrol'       #same than command but without option

#
#
#######################################################


    now=$(date +%s)

    time_to_stop=$((now+T))

    rm /home/minieusouser/DATA/* -f # purge data files from Zynq

    ($cmd &) #change this with your command
    echo 'mecontrol is starting @ '$(date -d @$now)

    # (./ZipDemon.sh &)


    inner_time_stop=$((now+t))

    while((now<time_to_stop)); do

        sleep $S
	
        flag=$(pgrep -x $p_name)
        #echo $flag
	
        if ((flag==0)) then
	   
           echo 'mecontrol is not running'
           echo 'then: Starting mecontrol'
           ($cmd &)
           inner_time_stop=$((now+t))

        elif ((now> inner_time_stop)); then
            echo 'mecontrol is still working'
            echo 'then: kill and reactivate mecontrol'
            kill -9 $flag
        fi

        now=$(date +%s)
    done

    echo 'cicle ends @ '$(date -d @$now)
