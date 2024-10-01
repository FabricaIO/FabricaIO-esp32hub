/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* Contributors: Sam Groveman
*/
#pragma once
#include <PeriodicTasks.h>

/// @brief Virtual class to describe a periodic task
class PeriodicTask {
	public:
		/// @brief Holds the description of the task
		struct {
			/// @brief The name of the task to use
			std::string taskName;

			/// @brief The period, in ms, that should elapse before task is run
			long taskPeriod;
		} task_config;

		/// @brief The total amount of time elapsed since last task call
		long totalElapsed = 0;

		virtual void runTask(long elapsed) = 0;
		bool enableTask(bool enable);

	protected:
		bool taskPeriodTriggered(long elapsed);
};