#include "SensorManager.h"

// Initialize static variables
std::vector<Sensor*> SensorManager::sensors;
std::vector<SensorManager::measurement> SensorManager::measurements;

/// @brief Adds a sensor to the in-use sensors collection
/// @param sensor A pointer to the sensor to add
/// @return True on success
bool SensorManager::addSensor(Sensor* sensor) {
	sensors.push_back(sensor);
	return true; // Currently no way to fail this
}

/// @brief Calls the begin function on all the in-use sensors
/// @return True if all sensors started correctly
bool SensorManager::beginSensors() {
	for (auto const &s : sensors) {
		if (!s->begin()) {
			Logger.println("Could not start " + s->Description.name);
			return false;
		} else {
			Logger.println("Started " + s->Description.name);
		}
	}
	return true;
}

/// @brief Takes a measurement form each sensors and stores it in the Measurements object
/// @return True if each sensor completes a measurement successfully
bool SensorManager::takeMeasurement() {
	// Clears any old measurements
	measurements.clear();
	// Take measurements
	for (auto const &s : sensors) {
		if (!s->takeMeasurement()) {
			Logger.println("Error taking measurement from " + s->Description.name);
			return false;
		}
		// Add measurements to results
		for (int i = 0; i < s->Description.parameterQuantity; i++) {
			measurements.push_back(measurement {
				.name = s->Description.name,
				.parameter = s->Description.parameters[i],
				.value = s->values[i],
				.unit = s->Description.units[i]
			});
		}
	}
	return true;
}

/// @brief Gets a complete collection of the last measurements recorded by the sensors
/// @return A JSON string with all the measurements
String SensorManager::getLastMeasurement() {
	// Allocate the JSON document
	JsonDocument doc;
	// Create array of measurements
	JsonArray measurement_array = doc["measurements"].to<JsonArray>();
	// Add measurements to array
	for (int i = 0; i < measurements.size(); i++) {
		measurement_array[i]["name"] = measurements[i].name;
		measurement_array[i]["parameter"] = measurements[i].parameter;
		measurement_array[i]["value"] = measurements[i].value;
		measurement_array[i]["unit"] = measurements[i].unit;
	}
	String output;
	serializeJson(doc, output);
	return output;
}

/// @brief Retrieves the information on all available sensors and their parameters
/// @return A JSON string of the information
String SensorManager::getSensorInfo() {
	// Allocate the JSON document
	JsonDocument doc;
	// Create array of senors
	JsonArray sensor_array = doc["sensors"].to<JsonArray>();
	// Add sensor info to array
	for (int i = 0; i < sensors.size(); i++) {
		// Add sensor description to array
		sensor_array[i]["positionID"] = i;
		sensor_array[i]["description"]["name"] = sensors[i]->Description.name;
		sensor_array[i]["description"]["parameterQuantity"] = sensors[i]->Description.parameterQuantity;
		sensor_array[i]["description"]["type"] = sensors[i]->Description.type;
		sensor_array[i]["description"]["version"] = sensors[i]->Description.version;
		// Add parameters to array
		for (int j = 0; j < sensors[i]->Description.parameterQuantity; j++) {
			sensor_array[i]["parameters"][j]["name"] = sensors[i]->Description.parameters[j];
			sensor_array[i]["parameters"][j]["unit"] = sensors[i]->Description.units[j];
		}
	}
	String output;
	serializeJson(doc, output);
	return output;
}

/// @brief Gets all sensors
/// @return A vector with pointers to each sensor
std::vector<Sensor*> SensorManager::getSensors() {
	return sensors;
}


/// @brief Gets any available config settings for a sensor device
/// @param sensorPosID The position ID of the sensor
/// @return A JSON string of configurable settings or an empty string on failure
String SensorManager::getSensorConfig(int sensorPosID) {
	if (sensorPosID >= 0 && sensorPosID < sensors.size()) {
		return sensors[sensorPosID]->getConfig();
	} else {
		return "";
	}
}

/// @brief Gets any available config settings for a sensor device
/// @param sensorPosID The position ID of the sensor
/// @return A JSON string of configurable settings or an empty string on failure
String SensorManager::getSensorConfig(String sensorName) {
	return getSensorConfig(sensorNameToID(sensorName));
}

/// @brief Gets any available config settings for a sensor device
/// @param sensorPosID The position ID of the sensor
/// @param config A JSON string of the configuration
/// @return True on success
bool SensorManager::setSensorConfig(int sensorPosID, String config) {
	if (sensorPosID >= 0 && sensorPosID < sensors.size()) {
		return sensors[sensorPosID]->setConfig(config, true);
	} else {
		return false;
	}
}

/// @brief Gets all the current sensor versions
/// @return A JSON string of all sensor versions
String SensorManager::getSensorVersions() {
	String output;
	if (sensors.size() > 0) {
		// Allocate the JSON document
		JsonDocument doc;
		// Add versions to object
		for (const auto& s : sensors) {
			doc[s->Description.name] = s->Description.version;
		}
		serializeJson(doc, output);
	} else {
		output = "{}";
	}
	return output;	
}

/// @brief Used to calibrate sensor
/// @param sensorPosID The position ID of the sensor to calibrate
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> SensorManager::calibrateSensor(int sensorPosID, int step) {
	if (sensorPosID < 0 || sensorPosID >= sensors.size()) {
		Logger.println("sensorPosID out of range");
		return { Sensor::calibration_response::ERROR, "sensorPosID out of range" };
	}
	return sensors[sensorPosID]->calibrate(step);
}

/// @brief Turns the name of a sensor into its position ID
/// @param name The name of the sensor
/// @return The positionID of the sensor or -1 on failure
int SensorManager::sensorNameToID(String name) {
	int sensorPosID;
	try {
		auto index = std::find_if(sensors.begin(), sensors.end(), [name](Sensor* a) {return a->Description.name == name;});
		if (index == sensors.end()) {
			Logger.println("Actor not found");
			return -1;
		}
		sensorPosID = index - sensors.begin();
	} catch (const std::exception& e) {
		Logger.println("Sensor not found");
		return -1;
	}
	return sensorPosID;
}