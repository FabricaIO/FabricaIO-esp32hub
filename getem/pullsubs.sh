#!/usr/bin/env bash
# FabricaIO-esp32hub extra modules
# 

START_DIR=${PWD}
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# List of subs to pull
declare -a subs=( 
    "sensor-GenericAnalogInput" 
    "util-Webhook" 
    "actor-DFAutoPeristalticPump" 
    "actor-DFPeristalticPump" 
    "util-ParameterTrigger" 
    "sensor-DFSoilMoisture" 
    "sensor-DFMultiEnvironmental" 
    "sensor-DFGasSensor" 
    "sensor-DFCO2Sensor" 
    "sensor-DFAirquality" 
    "actor-TimerSwitch" 
    "actor-ResetButton" 
    "actor-LocalDataLogger" 
    "actor-GenericOutput" 
    "actor-DataTemplate" 
    "eventreceiver-LEDIndicator" 
    "sensor-DummySensor" 
    )

length=${#subs[@]}
echo "Updating $length repos..."

cd ${SCRIPT_DIR}/lib

for i in "${subs[@]}"
do
    repo="git@github.com:FabricaIO/$i.git"
    if [ -d "${i}" ]; then
        echo "${i} exists!  Updating"
        cd ${i} && git pull && cd ..
    else
        echo "${i} does not exist yet - cloning ${repo} now!"
        git clone ${repo}
    fi
done

cd ${START_DIR}

# You can access them using echo "${arr[0]}", "${arr[1]}" also
