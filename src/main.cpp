/*
* This project and associated original files are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman 
* Contributors: Sam Groveman
*/

#include <Arduino.h>
#include <Wire.h>
#include <Webserver.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ESP32Time.h>
#include <Storage.h>
#include <Configuration.h>
#include <EventBroadcaster.h>
#include <ActorManager.h>
#include <WebServer.h>
#include <WiFiConfig.h>
#include <SensorManager.h>
#include <PeriodicTasks.h>
#include <DeviceLoader.h>
#include <LogBroadcaster.h>

/// @brief Current firmware version
extern const String FW_VERSION = "0.5.0";

/// @brief Set to true when the POST finishes successfully
bool POSTSuccess = false;

/// @brief RTC object for getting/setting time
ESP32Time rtc;

/// @brief Broadcasts log messages
LogBroadcaster logger;

/// @brief AsyncWebServer object (passed to WfiFiConfig and WebServer)
AsyncWebServer server(80);

/// @brief Loads all actor, sensor, and event receiver devices
DeviceLoader loader(&rtc);

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
	if (!Logger.beginReceivers())	{
		Serial.println("Could not start all log receivers");
		while(true);
	}

	// Start event receivers
	if (!EventBroadcaster::beginReceivers())	{
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
	
	if (Configuration::currentConfig.WiFiClient) {
		// Configure WiFi client
		DNSServer dns;
		AsyncWiFiManager manager(&server, &dns);
		WiFiConfig configurator(&manager, Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
		configurator.connectWiFi();
		WiFi.setAutoReconnect(true);
		server.reset();
		// Set local time via NTP once connected
		configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer.c_str());
	} else {
		// Start AP
		WiFi.softAP(Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
	}
	
	/// @brief Webserver handling all requests
	Webserver webserver(&server, &rtc);

	// Clear server settings, just in case
	webserver.ServerStop();

	// Start the update server
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

	// Ready!
	Logger.println("Time: " + rtc.getDateTime());
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Ready);
	Logger.println("System ready!");
	POSTSuccess = true;
}

// Used for tracking time intervals for timed events
ulong current_mills = 0;
ulong previous_mills_task = 0;

// Used to automatically synchronize clock at regular intervals
ulong previous_millis_ntp = 0;

void loop() {
	current_mills = millis();
	if(Configuration::currentConfig.WiFiClient && Configuration::currentConfig.useNTP) {
		// Synchronize the time every 6 hours
		if (current_mills - previous_millis_ntp > 21600000) {
			Logger.println("Setting time by NTP");
			configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer.c_str());
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
