#pragma once
#include <Arduino.h>
#include <LogBroadcaster.h>
#include <time.h>
#include <sys/time.h>

/// @brief Provides a static interface to access the current time
class TimeInterface {
	public:
		static String getFormattedTime(String format);
		static String getDateTime(bool longDate = false);
		static void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms = 0);
		static void setTime(unsigned long epoch, int us = 0, bool reapplyOffset = false);
		static void setOffset(long offset, int daylight);
		static long getEpoch();
		static long getLocalEpoch();

	private:
		/// @brief Current GMT offset in seconds
		static long currentOffset;
		
		/// @brief Current daylight offset in seconds
		static int currentDaylight;
		
		/// @brief Build and apply TZ string with DST support
		static void setTimeZone(long offset, int daylight);
};
