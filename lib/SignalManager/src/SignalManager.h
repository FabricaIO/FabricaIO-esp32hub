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
#include <SignalReceiver.h>
#include <vector>
#include <queue>

/// @brief Receives and processes signals for signal receivers
class SignalManager {
	private:
		/// @brief Stores all the in-use signal receivers
		static std::vector<SignalReceiver*> receivers;

		/// @brief Queue to hold signals to be processed.
		static QueueHandle_t signalQueue;

		/// @brief Holds all payloads delivered with a signal
		static std::queue<String> payloads;

	public:
		static bool addReceiver(SignalReceiver* receiver);
		static bool beginReceivers();
		static bool addSignalToQueue(int receiverPosID, String signal, String payload = "");
		static bool addSignalToQueue(int receiverPosID, int signal, String payload = "");
		static std::tuple<bool, String> processSignalImmediately(int receiverPosID, String signal, String payload = "");
		static std::tuple<bool, String> processSignalImmediately(int receiverPosID, int signal, String payload = "");
		static String getReceiverInfo();
		static String getReceiverConfig(int receiverPosID);
		static bool setReceiverConfig(int receiverPosID, String config);
		static void signalProcessor(void* arg);
};