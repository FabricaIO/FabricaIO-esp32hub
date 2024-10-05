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
#include <LogReceiver.h>
#include <vector>

/// @brief Used to broadcast log messages to all receivers
class LogBroadcaster : public Print {
	public:
		LogBroadcaster();
		bool beginReceivers();
		bool addReceiver(LogReceiver* receiver);
		String getReceiverVersions();

	private:
		/// @brief Stores all event receivers
		std::vector<LogReceiver*> receivers;

		size_t write(uint8_t c);
		size_t write(const uint8_t *buffer, size_t size);
};

/// @brief Global log broadcaster
extern LogBroadcaster Logger;