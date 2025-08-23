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
#include <LogBroadcaster.h>

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
			
			/// @brief Username for the web interface
			String webUsername = "Fabrica";

			/// @brief Password for the web interface
			String webPassword = "Fabrica";

			/// @brief Whether to use NTP for time data
			bool useNTP = true;

			/// @brief The NTP update period in milliseconds
			uint32_t ntpUpdatePeriod = 21600000;

			/// @brief NTP server 1
			String ntpServer1 = "pool.ntp.org";

			/// @brief NTP server 2
			String ntpServer2 = "time.google.com";

			/// @brief NTP server 3
			String ntpServer3 = "time.windows.com";

			/// @brief Daylight savings time offset in seconds
			int daylightOffset_sec = 3600;

			/// @brief Offset from GMT in seconds
			long  gmtOffset_sec = -18000;

			/// @brief If the device is a WiFi client or AP
			bool WiFiClient = true;

			/// @brief SSID for configuration interface
			String configSSID = "ESP32Hub_Config";


			/// @brief Password for configuration interface
			String configPW = "ESP32Hub";

			/// @brief Hostname used by device
			String hostname = "FabricaIO";
			
			/// @brief Enable MDNS using hostname
			bool mdns = true;

			/// @brief Use HTTP digest auth instead of HTTP basic auth
			bool useDigestAuth = false;
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