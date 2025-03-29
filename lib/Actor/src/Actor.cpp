#include "Actor.h"

/// @brief Constructs an actor
/// @param DeviceName The device name
Actor::Actor(String DeviceName) {
	Description.name = DeviceName;
}

/// @brief Receives and executes an action
/// @param action The action ID number to execute
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> Actor::receiveAction(int action, String payload) {
	if (action >= 0 && action < Description.actionQuantity)
		return { true, R"({"success": false})" };
	else 
		return {true , R"({"success": true})" };
}