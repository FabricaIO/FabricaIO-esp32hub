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
		struct TaskConfig {
			private:
				/// @brief The name of the task to use
				std::string taskName;

				/// @brief Pointer to the parent PeriodicTask
				PeriodicTask* parent;

			public:

				/// @brief Constructs this struct 
				/// @param p Pointer to the parent class
				TaskConfig(PeriodicTask* p) {parent = p;}

				/// @brief The period, in ms, that should elapse before task is run
				ulong taskPeriod;
				
				/// @brief Gets the current task name
				/// @return The task name as a string
				std::string get_taskName() {return taskName;}

				/// @brief Sets a new task name, removing the task for safety. Task will need to be reenabled manually
				/// @param name The new task name
				void set_taskName(std::string name) {parent->enableTask(false); taskName = name;}
				
		} task_config;

		/// @brief The total amount of time elapsed since last task call
		ulong totalElapsed = 0;

		/// @brief Default constructor used to initialize struct
		PeriodicTask() : task_config(this) {}

		virtual void runTask(ulong elapsed) = 0;
		bool enableTask(bool enable);

	protected:
		bool taskPeriodTriggered(ulong elapsed);
};