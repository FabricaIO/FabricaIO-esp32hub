/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Sensor.h>
#include <vector>
#include <ArduinoJson.h>

class SensorManager {
	private:
		/// @brief Collects all the senors that are in use
		static std::vector<Sensor*> sensors;

	public:
		/// @brief Describes all info associated with a measurement
		struct measurement {
			/// @brief The name of the sensor
			String name;

			/// @brief The parameter being measured
			String parameter;

			/// @brief The value of the measurement
			double value;

			/// @brief The unit of the measurement
			String unit;
		};
		/// @brief Contains the most recently requested measurements
		static std::vector<measurement> measurements;

		static bool addSensor(Sensor* sensor);
		static bool beginSensors();
		static bool takeMeasurement();
		static String getLastMeasurement();
		static String getSensorInfo();
		static std::vector<Sensor*> getSensors();
		static String getSensorConfig(int sensorPosID);
		static String getSensorConfig(String sensorName);
		static bool setSensorConfig(int sensorPosID, String config);
		static String getSensorVersions();
		static std::tuple<Sensor::calibration_response, String> calibrateSensor(int sensorPosID, int step);
		static int sensorNameToID(String name);
};