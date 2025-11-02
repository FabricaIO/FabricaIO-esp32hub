#!/usr/bin/env bash
# FabricaIO-esp32hub extra modules

START_DIR=${PWD}
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# List of subs to pull
declare -a subs=(
    "actor-DataTemplate"
    "actor-DFAutoPeristalticPump"
    "actor-DFPeriodicPeristalticPump"
    "actor-DFPeristalticPump"
    "actor-GenericDACOutput"
    "actor-GenericOutput"
    "actor-InterruptActionTrigger"
    "actor-LEDPWM"
    "actor-LocalDataLogger"
    "actor-NeoPixelsController"
    "actor-PeriodicActionTrigger"
    "actor-SensorActionTrigger"
    "actor-SensorChangeTrigger"
    "actor-ResetButton"
    "actor-TimerSwitch"
    "actor-WebhookAction"
    "eventreceiver-LEDIndicator"
    "logreceiver-LocalDebugLogger"
    "logreceiver-SerialLogger"
    "logreceiver-USBCDCLogger"
    "sensor-ADS1X15"
    "sensor-BoschBME280"
    "sensor-DFAirQuality"
    "sensor-DFCO2Sensor"
    "sensor-DFGasSensor"
    "sensor-DFMultiEnvironmental"
	"sensor-DFpHSensor"
    "sensor-DFSoilMoisture"
    "sensor-DFWaterConductivity"
    "sensor-DummySensor"
    "sensor-GenericAnalogInput"
    "sensor-GenericDigitalInput"
    "sensor-MPU6050IMU"
    "sensor-PlantowerPMSx003"
    "sensor-VEML7700LightSensor"
    "util-DigitalInputTrigger"
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
