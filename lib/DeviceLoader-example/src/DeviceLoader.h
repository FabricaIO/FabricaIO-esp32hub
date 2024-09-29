/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <EventBroadcaster.h>
#include <SensorManager.h>
#include <ActorManager.h>
#include <ESP32Time.h>

/******** Put additional includes here ********/

/******** End includes ********/

/// @brief Allows for loading of sensor, actor, and receiver devices
class DeviceLoader {
	public:
		DeviceLoader(ESP32Time* RTC);
		bool LoadEventReceivers();
		bool LoadDevices();
		
		/******** Declare sensor, actor, and receiver objects here ********/

		/******** End sensor, actor, and receiver object declarations ********/

	private:
		/// @brief Pointer to an ESP32Time object to use
		ESP32Time* rtc;
};