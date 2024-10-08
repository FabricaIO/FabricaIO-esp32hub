#include "Actor.h"

/// @brief Sets up the signal receiver
/// @return True on success
bool Actor::begin() {
	return false;
}

/// @brief Receives and reacts to an action
/// @param action The signal ID number to react to
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> Actor::receiveAction(int action, String payload) {
	if (action >= 0 && action < Description.actionQuantity)
		return { true, R"({"success": false})" };
	else 
		return {true , R"({"success": true})" };
}