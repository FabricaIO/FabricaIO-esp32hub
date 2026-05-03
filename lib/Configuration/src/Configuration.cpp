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
		Logger.println("Creating settings directory");
		if (!Storage::createDir("/settings")) {
			Logger.println("Could not create settings directory");
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
		Logger.println("Could not load config file, or it doesn't exist. Defaults used.");
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
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		Logger.println("Defaults used");
		return false;
	}
	// Assign loaded values
	currentConfig.tasksEnabled = doc["tasksEnabled"].as<bool>();
	currentConfig.period = doc["period"].as<int>();
	currentConfig.webUsername = doc["webUsername"] | "Fabrica";
	currentConfig.webPassword = doc["webPassword"] | "Fabrica";
	currentConfig.useNTP = doc["useNTP"] | true;
	currentConfig.ntpUpdatePeriod = (doc["ntpUpdatePeriod"] | 360) * 60000; // Convert from minutes to milliseconds
	currentConfig.ntpServer1 = doc["ntpServer1"].as<String>();
	currentConfig.ntpServer2 = doc["ntpServer2"].as<String>();
	currentConfig.ntpServer3 = doc["ntpServer3"].as<String>();
	currentConfig.hostname = doc["hostname"].as<String>();
	currentConfig.daylightOffset_sec = doc["daylightOffset"].as<int>();
	currentConfig.gmtOffset_sec = doc["gmtOffset"].as<long>();
	currentConfig.WiFiClient = doc["WiFiClient"] | true;
	currentConfig.configSSID = doc["configSSID"].as<String>();
	currentConfig.configPW = doc["configPW"].as<String>();
	currentConfig.hostname = doc["hostname"].as<String>();
	currentConfig.mdns = doc["mdns"].as<bool>();
	currentConfig.useDigestAuth = doc["useDigestAuth"].as<bool>();
	if (currentConfig.WiFiClient && currentConfig.useNTP) {
		// Set local time via NTP
		configTime(
			0, // ESPTime library requires internal time to be UTC
			Configuration::currentConfig.daylightOffset_sec,
			Configuration::currentConfig.ntpServer1.c_str(),
			Configuration::currentConfig.ntpServer2.c_str(),
			Configuration::currentConfig.ntpServer3.c_str());
		Logger.println("Time set via NTP");
	} else {
		// ESPTime library can't handle DST, use built in functions
		setDST();
	}
	TimeInterface::setOffset(currentConfig.gmtOffset_sec);
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
		Logger.println("Could not write config file");
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
	doc["ntpUpdatePeriod"] = currentConfig.ntpUpdatePeriod / 60000; // Convert from milliseconds to minutes
	doc["ntpServer1"] = currentConfig.ntpServer1;
	doc["ntpServer2"] = currentConfig.ntpServer2;
	doc["ntpServer3"] = currentConfig.ntpServer3;
	doc["gmtOffset"] = currentConfig.gmtOffset_sec;
	doc["daylightOffset"] = currentConfig.daylightOffset_sec;
	doc["WiFiClient"] = currentConfig.WiFiClient;
	doc["configSSID"] = currentConfig.configSSID;
	doc["configPW"] = currentConfig.configPW;
	doc["hostname"] = currentConfig.hostname;
	doc["mdns"] = currentConfig.mdns;
	doc["useDigestAuth"] = currentConfig.useDigestAuth;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

void Configuration::setDST() {
	// Taken from https://raw.githubusercontent.com/espressif/arduino-esp32/refs/heads/master/cores/esp32/esp32-hal-time.c
	long daylight = Configuration::currentConfig.daylightOffset_sec;

	char cst[21] = {0};
	char cdt[21] = "DST";
	char tz[41] = {0};

	snprintf(cst, sizeof(cst), "UTC%ld", 0);
	if (daylight != 3600) {
		long tz_dst = -daylight;
		if (tz_dst % 3600) {
			snprintf(cdt, sizeof(cdt), "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
		} else {
			snprintf(cdt, sizeof(cdt), "DST%ld", tz_dst / 3600);
		}
	}
	snprintf(tz, sizeof(tz), "%s%s", cst, cdt);
	setenv("TZ", tz, 1);
	tzset();
}