#include "PeriodicTask.h"

/// @brief Enables or disables a periodic task
/// @param enable True enable
/// @return True on success
bool PeriodicTask::enableTask(bool enable) {
	if (enable) {
		return PeriodicTasks::addTask(task_config.taskName, std::bind(&PeriodicTask::runTask, this, std::placeholders::_1));
	} else {
		return PeriodicTasks::removeTask(task_config.taskName);
	}
}

/// @brief Checks if a task period has elapsed and the task should run
/// @param elapsed The time elapsed since last check
/// @return True if task should run
bool PeriodicTask::taskPeriodTriggered(ulong elapsed) {
	totalElapsed += elapsed;
	if (totalElapsed > task_config.taskPeriod) {
		totalElapsed = 0;
		return true;
	}
	return false;
}