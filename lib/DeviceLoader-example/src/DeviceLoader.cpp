#include "DeviceLoader.h"

/// @brief Creates a new Device Loader
/// @param RTC A pointer to the RTC object to use
DeviceLoader::DeviceLoader(ESP32Time* RTC) {
	rtc = RTC;
}

/// @brief Loads all event receivers
/// @return True on success
bool DeviceLoader::LoadReceivers() {

	/******** Add event receivers and loggers here ********/

	/******** End event receivers and loggers addition section ********/

	return true;
}

/// @brief Loads all sensor and actor devices
/// @return True on success
bool DeviceLoader::LoadDevices() {
	
	/******** Add senors and actors here ********/

	/******** End senor and actor addition section ********/

	return true;
}