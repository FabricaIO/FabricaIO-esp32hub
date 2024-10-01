#include "DeviceConfig.h"

/// @brief Gets any available config settings for the current device
/// @return A JSON string of configurable settings
String DeviceConfig::getConfig() {
	return "{}";
}

/// @brief Updates configuration of device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool DeviceConfig::setConfig(String config, bool save) {
	return true;
}

/// @brief Saves the current config to a JSON file
/// @return True on success
/// @param path The path to the config file to save
/// @param contents The contents to save in the file
bool DeviceConfig::saveConfig(String path, String contents) {
	return Storage::writeFile(path, getConfig());
}

/// @brief Checks for the existence of the config file. Creates necessary containing directories as needed during check
/// @param path The path to the config file
/// @return True if config file exists
bool DeviceConfig::checkConfig(String path) {
	if (!Storage::fileExists(path)) {
		// Check for, and create, directories
		size_t pos = 0;
		String path_builder = "";
		// Remove starting '/'
		path.remove(0,1);
		while ((pos = path.indexOf('/')) != -1) {
			path_builder += "/" + path.substring(0, pos);
			if (!Storage::fileExists(path_builder)) {
				Storage::createDir(path_builder);
			}
			path.remove(0, pos + 1);
		}
		return false;
	} else {
		return true;
	}
}