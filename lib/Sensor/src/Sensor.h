/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <DeviceConfig.h>
#include <LogBroadcaster.h>
#include <vector>

/// @brief Defines a generic sensor class for inheriting 
class Sensor : public DeviceConfig {
	public:
		/// @brief Possible results from a calibration call
		enum calibration_response {
			error,
			done,
			next
		};

		/// @brief Holds the description of the sensor
		struct {
			/// @brief The number of parameter this sensor measures
			int parameterQuantity;

			/// @brief The type of this sensor
			String type;

			/// @brief The name of this sensor
			String name;

			/// @brief The parameters being measured
			std::vector<String> parameters;

			/// @brief The units associated with each measurement parameter
			std::vector<String> units;
			
			/// @brief The version of the sensor code
			String version = "0.0.1";
		} Description;

		/// @brief Stores measured values
		std::vector<double> values;

		Sensor(String DeviceName);
		virtual bool begin() = 0;
		virtual bool takeMeasurement() = 0;
		virtual std::tuple<Sensor::calibration_response, String> calibrate(int step);
};