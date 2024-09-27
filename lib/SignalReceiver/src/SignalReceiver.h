/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <map>
#include <DeviceConfig.h>

/// @brief Defines a generic signal receiver class for inheriting 
class SignalReceiver: public DeviceConfig {
	public:
		/// @brief Holds the description of the device capable of receiving signals
		struct {
			/// @brief The number of signals this device can receive
			int signalQuantity;

			/// @brief The type of device this is
			String type;

			/// @brief The name of this device
			String name;
			
			/// @brief Contains of map of signals this device can receive and their ID numbers. Signal names must contain only alphanumeric and underscores, and contain at least one letter
			std::map<String, int> signals;

			/// @brief The ID of this device
			int id;
		} Description;

		virtual bool begin();
		virtual std::tuple<bool, String> receiveSignal(int signal, String payload = "");
};