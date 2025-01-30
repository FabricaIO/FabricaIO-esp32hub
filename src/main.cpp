/*
* This project and associated original files are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman 
* Contributors: Sam Groveman
*/

#include <Arduino.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Webserver.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Storage.h>
#include <Configuration.h>
#include <EventBroadcaster.h>
#include <ActorManager.h>
#include <WebServer.h>
#include <WiFiConfig.h>
#include <SensorManager.h>
#include <PeriodicTasks.h>
#include <PeriodicTasks.h>
#include <DeviceLoader.h>
#include <TimeInterface.h>
#include <LogBroadcaster.h>

/// @brief Current firmware version
extern const String FW_VERSION = "0.5.0";

/// @brief Set to true when the POST finishes successfully
bool POSTSuccess = false;

/// @brief Broadcasts log messages
LogBroadcaster logger;

/// @brief AsyncWebServer object (passed to WfiFiConfig and WebServer)
AsyncWebServer server(80);

/// @brief Webserver handling all requests
Webserver webserver(&server);

/// @brief Loads all actor, sensor, and event receiver devices
DeviceLoader loader;

/// @brief Handles disconnection from WiFi (adapted from https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#11)
/// @param event The event
/// @param info The info associated with the event
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
	Logger.print("WiFi lost connection. Reason: ");
	Logger.println(info.wifi_sta_disconnected.reason);
	Logger.println("Trying to Reconnect");
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
	WiFi.disconnect();
	if (WiFi.reconnect()) {
		// Stop webserver
		webserver.ServerStop();
		// Start webserver
		webserver.ServerStart();
		Logger.println("WiFi reconnected");
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Ready);
	}
}

void setup() {
	// Start serial
	Serial.begin(115200);
	Serial.print("Booting ESP32 sensor hub V");
	Serial.println(FW_VERSION);
	Serial.println();
	Serial.println("Designed and created by Sam Groveman (C) 2024");
	Serial.println();

	// Load all event and log receivers
	loader.LoadReceivers();

	// Start loggers
	if (!Logger.beginReceivers()) {
		Serial.println("Could not start all log receivers");
		while(true);
	}

	// Start event receivers
	if (!EventBroadcaster::beginReceivers()) {
		Logger.println("Could not start all event receivers");
		while(true);
	}

	// Show yellow during startup
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Starting);

	// Start storage
	if (!Storage::begin()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Logger.println("Could not start storage");
		while(true);
	}

	// Start configuration manager
	if (!Configuration::begin()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Logger.println("Could not start configuration manager");
		while(true);
	}

	// Set hostname
	WiFi.setHostname(Configuration::currentConfig.hostname.c_str());

	// Pre-configure WiFi
	WiFi.mode(WIFI_STA);
 	esp_wifi_set_ps(WIFI_PS_NONE);
	
	if (Configuration::currentConfig.WiFiClient) {
		// Configure WiFi client
		DNSServer dns;
		AsyncWiFiManager manager(&server, &dns);
		WiFiConfig configurator(&manager, Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
		configurator.connectWiFi();
		WiFi.setAutoReconnect(true);
		server.reset();
		server.end();
		// Attach handler for WiFi disconnected
		WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
	} else {
		// Start AP
		WiFi.softAP(Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
	}

	// Clear server settings, just in case
	webserver.ServerStop();

	// Start the webserver
	webserver.ServerStart();
	xTaskCreate(Webserver::RebootCheckerTaskWrapper, "Reboot Checker Loop", 1024, &webserver, 1, NULL);

	// Load all devices
	loader.LoadDevices();

	// Start sensors
	if (!SensorManager::beginSensors()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		while(true);
	}

	// Start actors
	if (!ActorManager::beginActors()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		while(true);
	}

	// Print the configured sensors, actors, and webhooks
	Logger.println(SensorManager::getSensorInfo());
	Logger.println(ActorManager::getActorInfo());

	// Start signal processor loop (8K of stack depth is probably overkill, but it does process potentially large JSON strings and we have the RAM, so better to be safe)
	xTaskCreate(ActorManager::actionProcessor, "Action Processor Loop", 8192, NULL, 1, NULL);

	// Set time via NTP if needed
	if (Configuration::currentConfig.WiFiClient && Configuration::currentConfig.useNTP) {
		configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer1.c_str(), Configuration::currentConfig.ntpServer2.c_str(), Configuration::currentConfig.ntpServer3.c_str());
	}
	// Give info to user
	Logger.println("Time: " + TimeInterface::getDateTime());
	Logger.print("IP Address: ");
	Logger.println(WiFi.localIP().toString());
	// Ready!
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Ready);
	Logger.println("System ready!");
	POSTSuccess = true;
}

// Used for tracking time intervals for timed events
ulong current_mills = 0;
ulong previous_mills_task = 0;
ulong previous_millis_ntp = 0;

void loop() {
	current_mills = millis();
	if(Configuration::currentConfig.WiFiClient && Configuration::currentConfig.useNTP) {
		// Synchronize the time every 6 hours
		if (current_mills - previous_millis_ntp > 21600000) {
			Logger.println("Setting time by NTP");
			configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer1.c_str(), Configuration::currentConfig.ntpServer2.c_str(), Configuration::currentConfig.ntpServer3.c_str());
			previous_millis_ntp = current_mills;
		}
	}
	if (Configuration::currentConfig.tasksEnabled) {
		// Perform tasks periodically
		if (current_mills - previous_mills_task > Configuration::currentConfig.period) {
			EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Running);
			PeriodicTasks::callTasks(current_mills - previous_mills_task);
			previous_mills_task = current_mills;
			delay(100);
			EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Ready);
		}
	}
	delay(100);
}
