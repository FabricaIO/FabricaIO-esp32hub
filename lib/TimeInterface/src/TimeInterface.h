/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ESP32Time: https://github.com/fbiego/ESP32Time
*
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <LogBroadcaster.h>
#include <ESP32Time.h>

/// @brief Provides a static interface to access the current time
class TimeInterface {
	public:
		static String getFormattedTime(String format);
		static String getDateTime(bool longDate = false);
		static void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms = 0);
		static void setTime(unsigned long epoch, int ms = 0);
		static void setOffset(long offset);
		static long getEpoch();
		static long getLocalEpoch();

	private:
		/// @brief Time object to use
		static inline ESP32Time rtc;
};