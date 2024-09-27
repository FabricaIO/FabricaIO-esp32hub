#include "PeriodicTasks.h"

// Initialize static variables
std::unordered_map<std::string, std::function<void(long)>> PeriodicTasks::tasks;

/// @brief Calls all periodic tasks
/// @param elapsed The time in ms since the previous call of callTasks
void PeriodicTasks::callTasks(long elapsed) {
	Serial.println("Running tasks...");
	for (const auto& task : tasks) {
		Serial.print("Running task ");
		Serial.println(task.first.c_str());
		task.second(elapsed);
	}
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
	if (!taskExists(name)) {
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