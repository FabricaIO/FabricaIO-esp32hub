/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <Storage.h>

/// @brief Used by device classes to inherit saving a local configuration
class DeviceConfig {
	public:
		virtual String getConfig();
		virtual bool setConfig(String config, bool save);
	protected:
		bool saveConfig(String path, String contents);
		bool checkConfig(String path);
};