#include "PeriodicTask.h"

/// @brief Enables or disables a periodic task
/// @param enable True enable
/// @return True on success
bool PeriodicTask::enableTask(bool enable) {
	if (enable) {
		return PeriodicTasks::addTask(TaskDescription.taskName, std::bind(&PeriodicTask::runTask, this, std::placeholders::_1));
	} else {
		return PeriodicTasks::removeTask(TaskDescription.taskName);
	}
}