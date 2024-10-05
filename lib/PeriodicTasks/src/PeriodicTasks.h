/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/
#pragma once
#include<Arduino.h>
#include <SensorManager.h>
#include <LogBroadcaster.h>
#include <unordered_map>

/// @brief Holds all and calls tasks at periodic intervals
class PeriodicTasks {
	public:
		static void callTasks(long elapsed);
		static bool taskExists(std::string name);
		static bool addTask(std::string, std::function<void(long)> callback);
		static bool removeTask(std::string);
		
	private:
		/// @brief Holds a mapping of task names to the functions to call
		static std::unordered_map<std::string, std::function<void(long)>> tasks;
};