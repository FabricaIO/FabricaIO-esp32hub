#include "EventBroadcaster.h"

// Initialize static variables
std::vector<EventReceiver*> EventBroadcaster::receivers;
SemaphoreHandle_t EventBroadcaster::receiverMutex =  xSemaphoreCreateMutex();
QueueHandle_t EventBroadcaster::eventQueue = xQueueCreate(10, sizeof(int));

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

	// Ensure event queue was created
	if (eventQueue == NULL) {
		eventQueue = xQueueCreate(10, sizeof(int));
		if (eventQueue == NULL) {
			return false;
		}
	}

	// Take mutex and being all receivers
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
	int event_value = (int)event;
	
	// Add event to queue with blocking
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

/// @brief Event processor task loop, processes all events in queue and broadcasts to receivers
/// @param arg Not used
void EventBroadcaster::eventProcessor(void* arg) {
    int event;
    while(true) {
        // Process all events in the queue
        while (xQueueReceive(eventQueue, &event, pdMS_TO_TICKS(10)) == pdTRUE)
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
                // If we can't get the mutex, put the event back and try again
                xQueueSend(eventQueue, &event, 0);
            }
        }
        delay(5);  // Yield to allow other tasks to run
    }
}