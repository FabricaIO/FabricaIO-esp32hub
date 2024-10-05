/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ESPAsyncWiFiManager: https://github.com/alanswx/ESPAsyncWiFiManager
 * ArduinoJSON: https://arduinojson.org/
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <ESPAsyncWiFiManager.h>
#include <ArduinoJson.h>
#include <EventBroadcaster.h>
#include <LogBroadcaster.h>

class WiFiConfig {
	public:
		WiFiConfig(AsyncWiFiManager* WiFiManager, String SSID, String Password);
		void connectWiFi();

	private:
		/// @brief WiFi manager object
		AsyncWiFiManager* wifiManager;
		
		String ssid;
		String password;
		void configModeCallback(AsyncWiFiManager *myWiFiManager);
		void configModeEndCallback(AsyncWiFiManager *myWiFiManager);
};