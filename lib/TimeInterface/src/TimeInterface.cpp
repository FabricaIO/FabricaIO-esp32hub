#include "TimeInterface.h"

// Initialize static variables
ESP32Time TimeInterface::rtc;

/// @brief Get the current formatted time
/// @param format The CPP formatted time (https://cplusplus.com/reference/ctime/strftime/)
/// @return The formatted time as a string
String TimeInterface::getFormattedTime(String format) {
	return rtc.getTime(format);
}

/// @brief Gets the current date time
/// @param longDate True to use the long date time
/// @return The date time as a string
String TimeInterface::getDateTime(bool longDate) {
	return rtc.getDateTime(longDate);
}

/// @brief Sets the time using the passed parameters
/// @param sc seconds (0-59)
/// @param mn minute (0-29)
/// @param hr hour of day (0-23)
/// @param dy day of month (1-31)
/// @param mt month (1-12)
/// @param yr year (for digit)
/// @param ms microseconds (optional)
void TimeInterface::setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms) {
	rtc.setTime(sc, mn, hr, dy, mt, yr, ms);
}


/// @brief Sets the time using time since Unix epoch
/// @param epoch Epoch time in seconds
/// @param ms microseconds (optional)
void TimeInterface::setTime(unsigned long epoch, int ms) {
	rtc.setTime(epoch, ms);
}

/// @brief Sets the GMT timezone offset
/// @param offset The offset in seconds (e.g. 3600 for one hour)
void TimeInterface::setOffset(long offset) {
	rtc.offset = offset;
}

/// @brief Gets the current epoch seconds with the current timezone offset
/// @return The seconds since the Unix epoch
long TimeInterface::getEpoch() {
	return rtc.getEpoch();
}

/// @brief Gets the current epoch in seconds without timezone offset
/// @return The seconds since the Unix epoch
long TimeInterface::getLocalEpoch() {
	return rtc.getLocalEpoch();
}