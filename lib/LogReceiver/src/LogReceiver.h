/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
*
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>

class LogReceiver {
	public:
		/// @brief Contains a description of this receiver
		struct {
			/// @brief The name of the receiver
			String name;
			
			/// @brief The code version of this receiver
			String version = "0.01";
		} Description;

		virtual bool begin();
		virtual bool receiveMessage(String message) = 0;
		virtual bool receiveChar(char c) = 0;
};