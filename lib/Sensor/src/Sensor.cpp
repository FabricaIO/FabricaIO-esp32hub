#include "Sensor.h"

/// @brief Constructs a sensor device
/// @param DeviceName The name of the device
Sensor::Sensor(String DeviceName) {
	Description.name = DeviceName;
}

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> Sensor::calibrate(int step) {
	return { Sensor::calibration_response::error, "No calibration method" };
}