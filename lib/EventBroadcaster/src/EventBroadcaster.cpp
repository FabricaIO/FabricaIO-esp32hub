#include "EventBroadcaster.h"

// Initialize static variables
std::vector<EventReceiver*> EventBroadcaster::receivers;

/// @brief Begins on the subscribed receivers
/// @return True on success
bool EventBroadcaster::beginReceivers() {
	for (const auto& r : EventBroadcaster::receivers) {
		if (!r->begin()) {
			return false;
		}
	}
	return true;
}

/// @brief Broadcasts an event to all subscribed receivers
/// @param event The event to broadcast
/// @return True on success
bool EventBroadcaster::broadcastEvent(Events event) {
	for (const auto& r : EventBroadcaster::receivers) {
		if (!r->receiveEvent((int)event)) {
			return false;
		}
	}
	return true;
}

/// @brief Subscribes a receiver to the events
/// @param receiver A pointer to the receiver
/// @return True on success
bool EventBroadcaster::addReceiver(EventReceiver* receiver) {
	EventBroadcaster::receivers.push_back(receiver);
	return true; // Currently no way to fail this
}

/// @brief Get the versions of all connected receivers
/// @return A JSON string of all the versions
String EventBroadcaster::getReceiverVersions() {
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