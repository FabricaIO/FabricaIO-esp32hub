#!/usr/bin/env bash
# FabricaIO-esp32hub extra modules
# 

START_DIR=${PWD}
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# List of subs to pull
declare -a subs=(
    "actor-DataTemplate"
    "actor-DFAutoPeristalticPump" 
    "actor-DFPeristalticPump"
    "actor-GenericOutput"
    "actor-LocalDataLogger"
    "actor-ResetButton"
    "actor-TimerSwitch"
    "eventreceiver-LEDIndicator"
    "logreceiver-LocalDebugLogger"
    "logreceiver-SerialLogger"
    "sensor-BoschBME280"
    "sensor-DFAirquality"
    "sensor-DFCO2Sensor"
    "sensor-DFGasSensor"
    "sensor-DFMultiEnvironmental"
    "sensor-DFSoilMoisture"
    "sensor-DummySensor"
    "sensor-GenericAnalogInput"
    "sensor-PlantowerPMSx003"
    "util-ParameterTrigger"
    "util-SunsetSunrise"
    "util-Webhook"
    )

length=${#subs[@]}
echo "Updating $length repos..."

cd ${SCRIPT_DIR}/../lib

for i in "${subs[@]}"
do
    repo="git@github.com:FabricaIO/$i.git"
    if [ -d "${i}" ]; then
        echo "${i} exists!  Updating"
        cd ${i} && git pull
        cd ${SCRIPT_DIR}/../lib
    else
        echo "${i} does not exist yet - cloning ${repo} now!"
        git clone ${repo}
    fi
done

cd ${START_DIR}

# You can access them using echo "${arr[0]}", "${arr[1]}" also
