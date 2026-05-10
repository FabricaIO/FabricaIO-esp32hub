#include "TimeInterface.h"
#include <cstdlib>
#include <cstring>

// Initialize static variables
long TimeInterface::currentOffset = 0;
int TimeInterface::currentDaylight = 0;

/// @brief Get the current formatted time
/// @param format The C formatted time string (https://cplusplus.com/reference/ctime/strftime/)
/// @return The formatted time as a string or invalid on error
String TimeInterface::getFormattedTime(String format) {
	time_t now = time(nullptr);
	struct tm* timeinfo = localtime(&now);
	
	if (timeinfo == nullptr) {
		return "Invalid";
	}
	
	char buffer[256];
	strftime(buffer, sizeof(buffer), format.c_str(), timeinfo);
	return String(buffer);
}

/// @brief Gets the current date time
/// @param longDate True to use the long date time format
/// @return The date time as a string
String TimeInterface::getDateTime(bool longDate) {
	if (longDate) {
		// Format: "Monday, January 01, 2024 23:59:59"
		return getFormattedTime("%A, %B %d, %Y %H:%M:%S");
	} else {
		// Format: "2024-03-30 23:59:59"
		return getFormattedTime("%Y-%m-%d %H:%M:%S");
	}
}

/// @brief Sets the time using the passed parameters
/// @param sc seconds (0-59)
/// @param mn minute (0-59)
/// @param hr hour of day (0-23)
/// @param dy day of month (1-31)
/// @param mt month (0-11)
/// @param yr year (4 digit)
/// @param ms microseconds (optional)
void TimeInterface::setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms) {
	struct tm timeinfo = {};
	timeinfo.tm_sec = sc;
	timeinfo.tm_min = mn;
	timeinfo.tm_hour = hr;
	timeinfo.tm_mday = dy;
	timeinfo.tm_mon = mt;  
	timeinfo.tm_year = yr - 1900;  // tm_year is years since 1900
	timeinfo.tm_isdst = -1;        // Let mktime determine DST
	
	// Convert to epoch
	time_t epoch = mktime(&timeinfo);
	
	// Set system time
	timeval tv = {epoch, ms * 1000};  // Convert microseconds to milliseconds
	settimeofday(&tv, nullptr);
}

/// @brief Sets the time using time since Unix epoch
/// @param epoch Epoch time in seconds
/// @param us microseconds (optional)
/// @param Reapply Applies timezone offsets (default false)
void TimeInterface::setTime(unsigned long epoch, int us, bool reapplyOffset) {
	timeval tv = {(time_t)epoch, us};
	settimeofday(&tv, nullptr);
	if (reapplyOffset) {
		setTimeZone(-currentOffset, currentDaylight);
	}
}

/// @brief Sets the GMT timezone offset with DST support
/// @param offset The standard timezone offset in seconds (e.g. -18000 for EST)
/// @param daylight The daylight saving offset in seconds (typically 3600 for 1 hour)
void TimeInterface::setOffset(long offset, int daylight) {
	currentOffset = offset;
	currentDaylight = daylight;
	setTimeZone(-offset, daylight);
}

/// @brief Gets the current epoch seconds with the current timezone offset applied
/// @return The seconds since the Unix epoch, adjusted by timezone offset
long TimeInterface::getEpoch() {
	time_t now = time(nullptr);
	return (long)(now - currentOffset);
}

/// @brief Gets the current epoch in seconds without timezone offset
/// @return The seconds since the Unix epoch (UTC)
long TimeInterface::getLocalEpoch() {
	return (long)time(nullptr);
}

/// @brief Build and apply the TZ string with DST support
/// @param offset The standard timezone offset in seconds (negative for east)
/// @param daylight The daylight saving offset in seconds
/// @note Taken from https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-time.c
void TimeInterface::setTimeZone(long offset, int daylight) {
	char cst[21] = {0};
	char cdt[21] = "DST";
	char tz[41] = {0};
	if (offset % 3600) {
		snprintf(cst, sizeof(cst), "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
	} else {
		snprintf(cst, sizeof(cst), "UTC%ld", offset / 3600);
	}
	if (daylight != 3600) {
		long tz_dst = offset - daylight;
		if (tz_dst % 3600) {
			snprintf(cdt, sizeof(cdt), "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
		} else {
			snprintf(cdt, sizeof(cdt), "DST%ld", tz_dst / 3600);
		}
	}
	snprintf(tz, sizeof(tz), "%s%s", cst, cdt);
	setenv("TZ", tz, 1);
	tzset();
}
