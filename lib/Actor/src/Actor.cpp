#include "Actor.h"

/// @brief Sets up the actor
/// @return True on success
bool Actor::begin() {
	if (checkConfig("/settings/act/" + Description.name + ".json")) {
		return setConfig(Storage::readFile("/settings/act/" + Description.name + ".json"), false);
	}
	return true;
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

/// @brief Gets default actor config (just device name)
/// @return A JSON string of hte config
String Actor::getConfig() {
	// Allocate JSON document
	JsonDocument doc;

	// Name is only setting for default config
	doc["name"] = Description.name;

	// Allocate string
	String output;
	serializeJson(doc, output);
	return output;
}

/// @brief Sets default actor config (just device name)
/// @param config A JSON string of the config
/// @param save True to save to storage
/// @return True on success
bool Actor::setConfig(String config, bool save) {
	// Remove previous file
	if (checkConfig("/settings/act/" + Description.name + ".json")) {
		Storage::deleteFile("/settings/act/" + Description.name + ".json");
	}
	// Allocate the JSON document
	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	// Assign loaded values
	Description.name = doc["Name"].as<String>();
	if (save) {
		return saveConfig("/settings/act/" + Description.name + ".json", getConfig());
	}
	return true;
}