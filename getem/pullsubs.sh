# FabricaIO-esp32hub extra modules
# 

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

cd ../lib

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

cd ..

# You can access them using echo "${arr[0]}", "${arr[1]}" also
