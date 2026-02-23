#include "LogBroadcaster.h"

// Initialize static variables
QueueHandle_t LogBroadcaster::messageQueue = xQueueCreate(15, sizeof(String*));

/// @brief Global log broadcaster definition
LogBroadcaster Logger;

/// @brief Create a new log broadcaster
LogBroadcaster::LogBroadcaster() {
	receiverMutex = xSemaphoreCreateMutex();
	if (messageQueue == NULL) {
		messageQueue = xQueueCreate(15, sizeof(String*));
	}
}

/// @brief Begins on the subscribed receivers
/// @return True on success
bool LogBroadcaster::beginReceivers() {
	// Ensure mutex was created
	if (receiverMutex == NULL) {
		receiverMutex = xSemaphoreCreateMutex();
		if (receiverMutex == NULL) {
			return false;
		}
	}
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		return false;
	}
	for (const auto& r : receivers) {
		if (!r->begin()) {
			xSemaphoreGive(receiverMutex);
			return false;
		}
	}
	xSemaphoreGive(receiverMutex);
	return true;
}

/// @brief Subscribes a receiver to the log messages
/// @param receiver A pointer to the receiver
/// @return True on success
bool LogBroadcaster::addReceiver(LogReceiver* receiver) {
	if (xSemaphoreTake(receiverMutex, pdMS_TO_TICKS(5000)) == pdFALSE) {
		return false;
	}
	LogBroadcaster::receivers.push_back(receiver);
	xSemaphoreGive(receiverMutex);
	return true; // Currently no way to fail this
}

/// @brief Get the versions of all connected receivers
/// @return A JSON string of all the versions
String LogBroadcaster::getReceiverVersions() {
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

/// @brief Writes a char to all receivers
/// @param c The char to write
/// @return The number of bytes written (1)
size_t LogBroadcaster::write(uint8_t c) {
	String* message = new String((char)c);

	// Add message to queue
	if (xQueueSend(messageQueue, &message, pdMS_TO_TICKS(10000)) != pdTRUE) {
		delete message;
		return 0;
	}
	return 1;
}

/// @brief Converts a char* array buffer to a String and sends it to all receivers
/// @param buffer The buffer to write
/// @param size The size of the buffer
/// @return The number of bytes written
size_t LogBroadcaster::write(const uint8_t *buffer, size_t size) {
	String* message = new String((char*)buffer);

	// Add message to queue
	if (xQueueSend(messageQueue, &message, pdMS_TO_TICKS(10000)) != pdTRUE) {
		delete message;
		return 0;
	}
	return size;
}

/// @brief Message processor task loop, processes all messages in queue and sends to receivers
/// @param arg Not used
void LogBroadcaster::messageProcessor(void* arg) {
    String* message;
    while(true) {
        // Process all messages in the queue
        while (xQueueReceive(messageQueue, &message, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Take mutex before accessing receivers
            if (xSemaphoreTake(Logger.receiverMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                try {
                    for (const auto& r : Logger.receivers) {
                        if (!r->receiveMessage(*message)) {
                            break;  // Stop if a receiver fails
                        }
                    }
                    delete message;
                }
                catch (...) {
                    delete message;
                }
                xSemaphoreGive(Logger.receiverMutex);
            } else {
                // If we can't get the mutex, put the message back and try again
                xQueueSend(messageQueue, &message, 0);
            }
        }
        delay(5);  // Yield to allow other tasks to run
    }
}