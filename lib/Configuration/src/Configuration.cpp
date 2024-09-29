#include "Configuration.h"

// Initialize static variables
String Configuration::file;
Configuration::config Configuration::currentConfig;

/// @brief Starts a configuration manager
/// @param File Name of file used to store the configuration (placed in "/settings" directory)
/// @return True on success
bool Configuration::begin(String File) {
	file = "/settings/" + File;
	if (!Storage::fileExists("/settings")) {
		Serial.println("Creating settings directory");
		if (!Storage::createDir("/settings")) {
			Serial.println("Could not create settings directory");
			return false;
		}
	}
	return loadConfig();
}

/// @brief Deserializes JSON from config file and applies it to current config
/// @return True on success
bool Configuration::loadConfig() {
	String json_string = Storage::readFile(file);
	if (json_string == "") {
		Serial.println("Could not load config file, or it doesn't exist. Defaults used.");
		json_string = configToJSON();
	}
	return updateConfig(json_string);
}

/// @brief Updates the current config to match a JSON string of settings
/// @param config The config JSON string to use
/// @return True on success
bool Configuration::updateConfig(String config) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		Serial.println("Defaults used");
		return false;
	}
	// Assign loaded values
	currentConfig.tasksEnabled = doc["tasksEnabled"].as<bool>();
	currentConfig.period = doc["period"].as<int>();
	currentConfig.webUsername = doc["webUsername"] | "Fabrica";
	currentConfig.webPassword = doc["webPassword"] | "Fabrica";
	currentConfig.useNTP = doc["useNTP"] | true;;
	currentConfig.hostname = doc["hostname"].as<String>();
	currentConfig.daylightOffset_sec = doc["gmtOffset"].as<int>();
	currentConfig.gmtOffset_sec = doc["daylightOffset"].as<long>();
	currentConfig.WiFiClient = doc["WiFiClient"] | true;
	currentConfig.configSSID = doc["configSSID"].as<String>();
	currentConfig.configPW = doc["configPW"].as<String>();
	currentConfig.hostname = doc["hostname"].as<String>();
	if (currentConfig.WiFiClient) {
		// Set local time via NTP
		configTime(currentConfig.gmtOffset_sec, currentConfig.daylightOffset_sec, currentConfig.ntpServer.c_str());
		Serial.println("Time set via NTP");
	}
	return true;
}

/// @brief  Saves the current config to a JSON file
/// @return True on success
bool Configuration::saveConfig() {
	return saveConfig(configToJSON());
}

/// @brief Saves a string of config settings to config file. Does not apply the settings without a call to loadSettings()
/// @param config A complete and properly formatted JSON string of all the settings
/// @return True on success
bool Configuration::saveConfig(String config) {
	if(!Storage::writeFile(file, config)) {
		Serial.println("Could not write config file");
		return false;
	}
	return true;
}

/// @brief Gets the current configuration
/// @return The configuration as a JSON string
String Configuration::getConfig() {
	return configToJSON();
}

/// @brief Converts the current configuration to a JSON string
/// @return The configuration as a JSON string
String Configuration::configToJSON() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["tasksEnabled"] = currentConfig.tasksEnabled;
	doc["period"] = currentConfig.period;
	doc["webUsername"] = currentConfig.webUsername;
	doc["webPassword"] = currentConfig.webPassword;
	doc["useNTP"] = currentConfig.useNTP;
	doc["ntpServer"] = currentConfig.ntpServer;
	doc["gmtOffset"] = currentConfig.gmtOffset_sec;
	doc["daylightOffset"] = currentConfig.daylightOffset_sec;
	doc["WiFiClient"] = currentConfig.WiFiClient;
	doc["configSSID"] = currentConfig.configSSID;
	doc["configPW"] = currentConfig.configPW;
	doc["hostname"] = currentConfig.hostname;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}