#include "PeriodicTasks.h"

// Initialize static variables
std::unordered_map<std::string, std::function<void(long)>> PeriodicTasks::tasks;
SemaphoreHandle_t PeriodicTasks::taskMutex = NULL;

/// @brief Starts the periodic task controller
/// @return True on success
bool PeriodicTasks::begin() {
	if (taskMutex == NULL) {
		taskMutex = xSemaphoreCreateMutex();
		if (taskMutex == NULL) {
			return false;
		}
	}
	return true;
}

/// @brief Calls all periodic tasks
/// @param elapsed The time in ms since the previous call of callTasks
/// @return True on success
bool PeriodicTasks::callTasks(long elapsed) {
	// Make a snapshot copy of the tasks under lock to avoid iterator invalidation
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(1000)) == pdFALSE) {
		Logger.println("Timed out taking task mutex");
		return false;
	}
	auto snapshot = tasks;
	xSemaphoreGive(taskMutex);

	Logger.println("Running tasks...");
	// Ensure sensor measurement success
	if (!SensorManager::takeMeasurement()) {
		Logger.println("Sensor measurement failed");
		return false;
	}
	for (const auto& task : snapshot) {
		Logger.print("Running task ");
		Logger.println(task.first.c_str());
		try {
			task.second(elapsed);
		} catch (const std::exception &e) {
			Logger.print("Task threw exception: ");
			Logger.println(e.what());
		} catch (...) {
			Logger.println("Task threw unknown exception");
		}
	}
	return true;
}

/// @brief Checks to see if a task currently exists
/// @param name The name of the task to check for
/// @return True if the task exists
bool PeriodicTasks::taskExists(std::string name) {
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(1000)) == pdFALSE) { 
		return false;
	}
	bool exists = tasks.find(name) != tasks.end();
	xSemaphoreGive(taskMutex);
	return exists;
}

/// @brief Adds a function to the collection of periodic tasks
/// @param name The name to give the task
/// @param callback A pointer to the function callback
/// @return True on success
bool PeriodicTasks::addTask(std::string name, std::function<void(long)> callback) {
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(1000)) == pdFALSE) {
		return false;
	}
	// Only add if it doesn't already exist
	if (tasks.find(name) == tasks.end()) {
		Logger.print("Adding task ");
		Logger.println(name.c_str());
		bool res = tasks.emplace(name, callback).second;
		xSemaphoreGive(taskMutex);
		return res;
	}
	xSemaphoreGive(taskMutex);
	return true;
}

/// @brief Removes a task from the collection of periodic tasks
/// @param name The name of the task to remove
/// @return True on success
bool PeriodicTasks::removeTask(std::string name) {
	if (xSemaphoreTake(taskMutex, pdMS_TO_TICKS(1000)) == pdFALSE) {
		return false;
	}
	if (tasks.find(name) != tasks.end()) {
		size_t removed = tasks.erase(name);
		xSemaphoreGive(taskMutex);
		return removed > 0;
	}
	xSemaphoreGive(taskMutex);
	return true;
}