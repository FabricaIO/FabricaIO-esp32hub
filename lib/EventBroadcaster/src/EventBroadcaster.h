/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
*
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EventReceiver.h>
#include <vector>

/// @brief Broadcasts device events
class EventBroadcaster {
	private:
		/// @brief Stores all event receivers
		static std::vector<EventReceiver*> receivers;

	public:
		/// @brief Stores possible events to raise
		enum Events { Clear, Running, Ready, Starting, WifiConfig, Updating, Rebooting, Error };

		static bool beginReceivers();
		static bool broadcastEvent(Events event);
		static bool addReceiver(EventReceiver* receiver);
		static String getReceiverVersions();
};
