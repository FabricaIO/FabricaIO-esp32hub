#include "SignalReceiver.h"

/// @brief Sets up the signal receiver
/// @return True on success
bool SignalReceiver::begin() {
	return false;
}

/// @brief Receives and reacts to a signal
/// @param signal The signal ID number to react to
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> SignalReceiver::receiveSignal(int signal, String payload) {
	if (signal >= 0 && signal < Description.signalQuantity)
		return { true, R"({"success": false})" };
	else 
		return {true , R"({"success": true})" };
}