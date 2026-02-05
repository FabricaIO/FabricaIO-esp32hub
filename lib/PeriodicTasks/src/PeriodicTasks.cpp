#include "PeriodicTasks.h"

// Initialize static variables
std::unordered_map<std::string, std::function<void(long)>> PeriodicTasks::tasks;

/// @brief Calls all periodic tasks
/// @param elapsed The time in ms since the previous call of callTasks
/// @return True on success
bool PeriodicTasks::callTasks(long elapsed) {
	Logger.println("Running tasks...");
	// Ensure sensor measurement success
	if (!SensorManager::takeMeasurement()) {
		return false;
	}
	for (const auto& task : tasks) {
		Logger.print("Running task ");
		Logger.println(task.first.c_str());
		task.second(elapsed);
	}
	return true;
}

/// @brief Checks to see if a task currently exists
/// @param name The name of the task to check for
/// @return True if the task exists
bool PeriodicTasks::taskExists(std::string name) {
	return tasks.find(name) != tasks.end();
}

/// @brief Adds a function to the collection of periodic tasks
/// @param name The name to give the task
/// @param callback A pointer to the function callback
/// @return True on success
bool PeriodicTasks::addTask(std::string name, std::function<void(long)> callback) {
	Logger.println("Adding tasks...");
	if (!taskExists(name)) {
		Logger.print("Adding task ");
		Logger.println(name.c_str());
		return tasks.emplace(name, callback).second;
	}
	return true;
}

/// @brief Removes a task from the collection of periodic tasks
/// @param name The name of the task to remove
/// @return True on success
bool PeriodicTasks::removeTask(std::string name) {
	if (taskExists(name)) {
		return tasks.erase(name) > 0;
	}
	return true;
}