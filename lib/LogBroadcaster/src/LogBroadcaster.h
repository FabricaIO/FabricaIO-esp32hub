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
#include <memory>

/// @brief Used to broadcast log messages to all receivers
class LogBroadcaster : public Print {
	public:
		/// @brief True when there are no log receivers
		static bool noReceivers;

		/// @brief Task handle for message processor loop
		static TaskHandle_t loggerHandle;

		/// @brief Mutex for protecting access to task handle
		static SemaphoreHandle_t taskMutex;
		
		LogBroadcaster();
		bool beginReceivers();
		bool addReceiver(LogReceiver* receiver);
		String getReceiverVersions();
		static void messageProcessor(void* arg);

	private:
		/// @brief Stores all event receivers
		static std::vector<LogReceiver*> receivers;

		/// @brief Mutex for thread safety when modifying receivers.
		/// @note Not strictly necessary given strict startup order, but doesn't hurt
		SemaphoreHandle_t receiverMutex = NULL;

		/// @brief Queue to hold messages to be processed
		static QueueHandle_t messageQueue;

		size_t write(uint8_t c);
		size_t write(const uint8_t *buffer, size_t size);
		size_t addMessageToQueue(String* message, size_t size = 1);
};

/// @brief Global log broadcaster
extern LogBroadcaster Logger;