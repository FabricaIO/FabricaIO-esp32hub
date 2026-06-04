#include "LogBroadcaster.h"

// Initialize static variables
QueueHandle_t LogBroadcaster::messageQueue = xQueueCreate(15, sizeof(String*));
bool LogBroadcaster::noReceivers = true;
TaskHandle_t LogBroadcaster::loggerHandle = nullptr;
SemaphoreHandle_t LogBroadcaster::taskMutex = xSemaphoreCreateMutex();
std::vector<LogReceiver*> LogBroadcaster::receivers;

/// @brief Global log broadcaster definition
LogBroadcaster Logger;

/// @brief Begins on the subscribed receivers
/// @return True on success
bool LogBroadcaster::beginReceivers() {
	// Ensure mutex was created
	if (taskMutex == NULL) {
		taskMutex = xSemaphoreCreateMutex();
		if (taskMutex == NULL) {
			return false;
		}
	}
	if (!receivers.empty()) {
		noReceivers = false;
		for (const auto& r : receivers) {
			if (!r->begin()) {
				return false;
			}
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
	return addMessageToQueue(message);
}

/// @brief Writes a char* array buffer to all receivers
/// @param buffer The buffer to write
/// @param size The size of the buffer
/// @return The number of bytes written
size_t LogBroadcaster::write(const uint8_t *buffer, size_t size) {
	String* message = new String((char*)buffer);
	return addMessageToQueue(message, size);
}

/// @brief Adds a message string to the queue
/// @param message The string pointer to add
/// @param size The size of the buffer
/// @return The number of bytes written
size_t LogBroadcaster::addMessageToQueue(String* message, size_t size) {
	if (noReceivers) {
		// Ignore message (return message size since this is not an error)
		delete message;
		return size;
	}
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(500)) == pdFALSE) {
		delete message;
		return 0;
	}
	TaskHandle_t loggerTaskCopy = loggerHandle;
	xSemaphoreGive(taskMutex);
	if(loggerTaskCopy == nullptr) {
		// Message processor loop not running when it should be
		delete message;
		return 0;
	}
	// Add message to queue
	if (xQueueSend(messageQueue, &message, pdMS_TO_TICKS(10000)) != pdTRUE) {
		delete message;
		// Could not add message to queue
		return 0;
	}
	return size;
}

/// @brief Message processor task loop, processes all messages in queue and sends to receivers
/// @param arg Not used
void LogBroadcaster::messageProcessor(void* arg) {
	if (noReceivers) {
		Serial.println("No log receivers, exiting log message processor");
		xSemaphoreTake(taskMutex, portMAX_DELAY);
		loggerHandle = nullptr;
		xSemaphoreGive(taskMutex);
		vTaskDelete(NULL);
		return;
	}
	String* message;
	// Process all messages in the queue
	while (xQueueReceive(messageQueue, &message, portMAX_DELAY) == pdTRUE) {
		try { // Try/catch is not a great solution here, should be improved
			for (const auto& r : Logger.receivers) {
				if (!r->receiveMessage(*message)) {
					Logger.printf("Error with log receiver %s", r->Description.name);
				}
			}
		}
		catch (...) {
			Logger.println("Exception in processing log message from queue");
		}
		delete message;
	}
	xSemaphoreTake(taskMutex, portMAX_DELAY);
	loggerHandle = nullptr;
	xSemaphoreGive(taskMutex);
	vTaskDelete(NULL);
}