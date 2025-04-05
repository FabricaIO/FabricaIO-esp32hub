/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <DeviceConfig.h>
#include <LogBroadcaster.h>
#include <map>

/// @brief Defines a generic actions class for inheriting 
class Actor : public DeviceConfig {
	public:
		/// @brief Holds the description of the device
		struct {
			/// @brief The number of actions this device can perform
			int actionQuantity;

			/// @brief The type of device this is
			String type;

			/// @brief The name of this device
			String name;
			
			/// @brief Contains of map of actions this device can perform and their ID numbers. Actor names must contain only alphanumeric and underscores, and contain at least one letter
			std::map<String, int> actions;

			/// @brief The version of the actor code
			String version = "0.0.1";
		} Description;

		Actor(String DeviceName);
		virtual bool begin() = 0;
		virtual std::tuple<bool, String> receiveAction(int action, String payload = "");
};