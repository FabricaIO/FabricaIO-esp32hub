#include "EventBroadcaster.h"

// Initialize static variables
std::vector<EventReceiver*> EventBroadcaster::receivers;

SemaphoreHandle_t EventBroadcaster::receiverMutex =  xSemaphoreCreateMutex();;

/// @brief Begins on the subscribed receivers
/// @return True on success
bool EventBroadcaster::beginReceivers() {
	// Ensure mutex was created
	if (receiverMutex == NULL) {
		receiverMutex = xSemaphoreCreateMutex();
		if (receiverMutex == NULL) {
			return false;
		}
	}
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		Logger.println("Event broadcaster failed to acquire mutex");
		return false;
	}
	for (const auto& r : EventBroadcaster::receivers) {
		if (!r->begin()) {
			return false;
		}
	}
	xSemaphoreGive(receiverMutex);
	return true;
}

/// @brief Broadcasts an event to all subscribed receivers
/// @param event The event to broadcast
/// @return True on success
bool EventBroadcaster::broadcastEvent(Events event) {
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		Logger.println("Event broadcaster failed to acquire mutex");
		return false;
	}
	for (const auto& r : EventBroadcaster::receivers) {
		if (!r->receiveEvent((int)event)) {
			return false;
		}
	}
	xSemaphoreGive(receiverMutex);
	return true;
}

/// @brief Subscribes a receiver to the events
/// @param receiver A pointer to the receiver
/// @return True on success
bool EventBroadcaster::addReceiver(EventReceiver* receiver) {
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		Logger.println("Event broadcaster failed to acquire mutex");
		return false;
	}
	EventBroadcaster::receivers.push_back(receiver);
	xSemaphoreGive(receiverMutex);
	return true;
}

/// @brief Get the versions of all connected receivers
/// @return A JSON string of all the versions
String EventBroadcaster::getReceiverVersions() {
	String output = "{}";
	if (receivers.size() > 0) {
		// Allocate the JSON document
		JsonDocument doc;
		// Add versions to object
		for (const auto& r : receivers) {
			doc[r->Description.name] = r->Description.version;
		}
		serializeJson(doc, output);
		
	}
	return output;
}