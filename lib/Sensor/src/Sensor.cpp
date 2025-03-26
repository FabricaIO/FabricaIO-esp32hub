#include "Sensor.h"

/// @brief Used to run initial setup and configure a sensor
/// @return True on success
bool Sensor::begin() {
	if (checkConfig("/settings/sen/" + Description.name + ".json")) {
		return setConfig(Storage::readFile("/settings/sen/" + Description.name + ".json"), false);
	}
	return true;
}

/// @brief Has as sensor take a measurement
/// @return True on success
bool Sensor::takeMeasurement() {
	return false;
}


/// @brief Gets default sensor config (just device name)
/// @return A JSON string of hte config
String Sensor::getConfig() {
	// Allocate JSON document
	JsonDocument doc;

	// Name is only setting for default config
	doc["Name"] = Description.name;

	// Allocate string
	String output;
	serializeJson(doc, output);
	return output;
}

/// @brief Sets default sensor config (just device name)
/// @param config A JSON string of the config
/// @param save True to save to storage
/// @return True on success
bool Sensor::setConfig(String config, bool save) {
	// Remove previous file
	if (checkConfig("/settings/sen/" + Description.name + ".json")) {
		Storage::deleteFile("/settings/sen/" + Description.name + ".json");
	}
	// Allocate the JSON document
	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	// Assign loaded values
	Description.name = doc["Name"].as<String>();
	if (save) {
		return saveConfig("/settings/sen/" + Description.name + ".json", getConfig());
	}
	return true;
}

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> Sensor::calibrate(int step) {
	return { Sensor::calibration_response::error, "No calibration method" };
}