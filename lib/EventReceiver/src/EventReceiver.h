/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>

/// @brief Receives device events
class EventReceiver {
	public:
		/// @brief Contains a description of this receiver
		struct {
			/// @brief The name of the receiver
			String name;
			
			/// @brief The code version of this receiver
			String version = "0.01";
		} Description;

		virtual bool begin();
		virtual bool receiveEvent(int event);
};