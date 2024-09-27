/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <ArduinoJson.h>
#include <Storage.h>

/// @brief Holds and manages the hub configuration
class Configuration {
	private:
		/// @brief Path to file used to store/load settings
		static String file;

		/// @brief Defines the configuration 
		typedef struct config {
			/// @brief Controls whether the sensor scheduled tasks are enabled
			bool tasksEnabled = false;
			
			/// @brief Controls the sampling period of the sensor hub
			int period = 10000;

			/// @brief NTP server
			String ntpServer = "pool.ntp.org";

			/// @brief Daylight savings time offset in seconds
			int daylightOffset_sec = 3600;

			/// @brief Offset from GMT in seconds
			long  gmtOffset_sec = -18000;

			bool WiFiClient = true;

			/// @brief SSID for configuration interface
			String configSSID = "SensorHub_Config";

			/// @brief Password for configuration interface
			String configPW = "ESP32Sensor";
		} config;

		static String configToJSON();

	public:
		/// @brief The currently used configuration
		static config currentConfig;

		static bool begin(String File = "config.json");
		static bool loadConfig();
		static bool updateConfig(String config);
		static bool saveConfig();
		static bool saveConfig(String config);
		static String getConfig();
};