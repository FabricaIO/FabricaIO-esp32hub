#include "LogBroadcaster.h"

/// @brief Global log broadcaster definition
LogBroadcaster Logger;

/// @brief Create a new log broadcaster
LogBroadcaster::LogBroadcaster() {
	Logger = *this;
}

/// @brief Begins on the subscribed receivers
/// @return True on success
bool LogBroadcaster::beginReceivers() {
	for (const auto& r : receivers) {
		if (!r->begin()) {
			return false;
		}
	}
	return true;
}

/// @brief Subscribes a receiver to the log messages
/// @param receiver A pointer to the receiver
/// @return True on success
bool LogBroadcaster::addReceiver(LogReceiver* receiver) {
	LogBroadcaster::receivers.push_back(receiver);
	return true; // Currently no way to fail this
}

/// @brief Get the versions of all connected receivers
/// @return A JSON string of all the versions
String LogBroadcaster::getReceiverVersions() {
	String output;
	if (receivers.size() > 0) {
		// Allocate the JSON document
		JsonDocument doc;
		// Add versions to object
		for (const auto& r : receivers) {
			doc[r->Description.name] = r->Description.version;
		}
		serializeJson(doc, output);
	} else {
		output = "{}";
	}
	return output;
}

/// @brief Writes a char to all receivers
/// @param c 
/// @return The number of bytes written (1)
size_t LogBroadcaster::write(uint8_t c) {
	for (const auto& r : receivers) {
		if (!r->receiveChar((char)c)) {
			return false;
		}
	}
	return 1;
}

/// @brief Converts a char* array buffer to a String and sends it to all receivers
/// @param buffer The buffer to write
/// @param size The size of the buffer
/// @return The number of bytes written
size_t LogBroadcaster::write(const uint8_t *buffer, size_t size) {
	String message = String((char*)buffer);
	for (const auto& r : receivers) {
		if (!r->receiveMessage(message)) {
			return false;
		}
	}
	return size;
}