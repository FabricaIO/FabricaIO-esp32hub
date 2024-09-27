/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

#pragma once

/// @brief Receives device events
class EventReceiver {
	public:
		virtual bool begin();
		virtual bool receiveEvent(int event);
};