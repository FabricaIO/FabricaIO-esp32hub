#include "EventBroadcaster.h"

// Initialize static variables
std::vector<EventReceiver*> EventBroadcaster::receivers;
SemaphoreHandle_t EventBroadcaster::receiverMutex =  xSemaphoreCreateMutex();
QueueHandle_t EventBroadcaster::eventQueue = xQueueCreate(10, sizeof(int));
TaskHandle_t EventBroadcaster::eventHandle = nullptr;
SemaphoreHandle_t EventBroadcaster::taskMutex = xSemaphoreCreateMutex();
bool EventBroadcaster::noReceivers = true;

/// @brief Begins the subscribed receivers
/// @return True on success
bool EventBroadcaster::beginReceivers() {
	// Ensure mutex was created
	if (receiverMutex == NULL) {
		receiverMutex = xSemaphoreCreateMutex();
		if (receiverMutex == NULL) {
			return false;
		}
	}

	// Ensure mutex was created
	if (taskMutex == NULL) {
		taskMutex = xSemaphoreCreateMutex();
		if (taskMutex == NULL) {
			return false;
		}
	}

	// Ensure event queue was created
	if (eventQueue == NULL) {
		eventQueue = xQueueCreate(10, sizeof(int));
		if (eventQueue == NULL) {
			return false;
		}
	}

	// Take mutex and begin all receivers
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		Logger.println("Event broadcaster failed to acquire mutex");
		return false;
	}
	if (!receivers.empty()) {
		noReceivers = false;
		for (const auto& r : EventBroadcaster::receivers) {
			if (!r->begin()) {
				return false;
			}
		}
	}
	xSemaphoreGive(receiverMutex);
	return true;
}

/// @brief Broadcasts an event to all subscribed receivers
/// @param event The event to broadcast
/// @return True on success
bool EventBroadcaster::broadcastEvent(Events event) {
	if (noReceivers) {
		// Discard event (this returns true because it's not an error)
		return true;
	}
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(500)) == pdFALSE) {
		return false;
	}
	TaskHandle_t eventTaskCopy = eventHandle;
	xSemaphoreGive(taskMutex);
		
	if(eventTaskCopy == NULL) {
		// Event processor loop not running when it should be
		return false;
	}
	int event_value = (int)event;
	// Add event to queue
	if (xQueueSend(eventQueue, &event_value, pdMS_TO_TICKS(10000)) != pdTRUE) {
		Logger.println("Failed to queue event");
		return false;
	}
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
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
		if (!receivers.empty()) {
			// Allocate the JSON document
			JsonDocument doc;
			// Add versions to object
			for (const auto& r : receivers) {
				doc[r->Description.name] = r->Description.version;
			}
			serializeJson(doc, output);
			
		}
	}
	return output;
}

/// @brief Event processor task loop, processes all events in queue and broadcasts to receivers
/// @param arg Not used
void EventBroadcaster::eventProcessor(void* arg) {
	if (receivers.empty()) {
		Logger.println("No event receivers, exiting event processor");
		xSemaphoreTake(taskMutex, portMAX_DELAY);
		eventHandle = nullptr;
		xSemaphoreGive(taskMutex);
		vTaskDelete(NULL);
		return;
	}
	int event;
	// Process all events in the queue
	while (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdTRUE)
	{
		// Take mutex before accessing receivers
		if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
			try {
				for (const auto& r : EventBroadcaster::receivers) {
					if (!r->receiveEvent(event)) {
						break;  // Stop if a receiver fails
					}
				}
			}
			catch (...) {
				Logger.println("Exception in processing event from queue");
			}
			xSemaphoreGive(receiverMutex);
		} else {
			// If mutex can't be taken, put the event back and try again
			if (xQueueSend(eventQueue, &event, pdMS_TO_TICKS(100)) == pdFAIL) {					 
				Logger.println("Log message dropped due to mutex contention");			
			}
		}
	}
	xSemaphoreTake(taskMutex, portMAX_DELAY);
	eventHandle = nullptr;
	xSemaphoreGive(taskMutex);
	vTaskDelete(NULL);
}