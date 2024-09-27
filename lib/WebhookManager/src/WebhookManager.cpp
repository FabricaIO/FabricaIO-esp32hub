#include "WebhookManager.h"

// Initialize static variables
std::vector<WebhookManager::Webhook_info> WebhookManager::webhooks;
String WebhookManager::config;

/// @brief Starts the webhook manager
/// @param configFile Name of config file
bool WebhookManager::begin(String configFile) {
	config = "/settings/" + configFile;
	return true;
}

/// @brief Loads webhooks from config file
/// @return True on success or if nothing has been configured yet
bool WebhookManager::loadWebhooks() {
	if (Storage::fileExists(config)) {
		// Attempt to load and read config file
		String json_string = Storage::readFile(config);
		if (json_string == "") {
			Serial.println("Could not load webhook config file");
			return false;
		}
		return updateWebhooks(json_string);
	}
	return true;	
}

/// @brief Updates current in-use webhooks
/// @param hooks A complete JSON string of all webhooks containing an array named "hooks"
/// @return True on success
bool WebhookManager::updateWebhooks(String hooks) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		return false;
	}
	// Clear old hooks
	webhooks.clear();
	// Add webhooks
	int i = 0;
	for (JsonObject hook : doc["hooks"].as<JsonArray>()) {
		std::map<String, String> headers;
		// Create header map
		for (JsonPair header : hook["headers"].as<JsonObject>()) {
			headers[header.key().c_str()] = header.value().as<String>();
		}
		// Add webhook to vector
		webhooks.push_back(Webhook_info {
			.positionID = i,
			.hook = std::shared_ptr<Webhook>(new Webhook(hook["url"].as<String>(), headers))
		});
		i++;
	}
	return true;
}

/// @brief  Saves the current webhooks to a JSON file
/// @return True on success
bool WebhookManager::saveWebhooks() {
	return saveWebhooks(hooksToJSON());
}

/// @brief Saves a string of webhooks to a config file. Does not apply the webhooks without a call to loadWebhooks()
/// @param hooks A complete and properly formatted JSON string of all the webhooks
/// @return True on success
bool WebhookManager::saveWebhooks(String hooks) { 
	if(!Storage::writeFile(config, hooks)) {
		Serial.println("Could not write webhooks config file");
		return false;
	}
	return true;
}

/// @brief Gets the current webhooks
/// @return The webhooks as a JSON string
String WebhookManager::getWebhooks() {
	return hooksToJSON();
}

/// @brief Sends a GET request with no parameters
/// @param PositionID The positionID (vector index) of the webhook
/// @return A JSON string with "code" as the response code and "response" as the response payload
String WebhookManager::fireGet(int PositionID) {
	String response;
	if (PositionID > 0 && PositionID < webhooks.size()) {
		response = webhooks[PositionID].hook->getRequest();
	} else {
		response = "{\"code\": 400, \"response\": \"Bad PositionID data\"}";
	}
	return response;
}

/// @brief Sends a GET request with parameters
/// @param PositionID The positionID (vector index) of the webhook
/// @param parameters A JSON string of parameter names and values
/// @return A JSON string with "code" as the response code and "response" as the response payload
String WebhookManager::fireGet(int PositionID, String Parameters) {
	String response;
	if (PositionID > 0 && PositionID < webhooks.size()) {
		response = webhooks[PositionID].hook->getRequest(Parameters);
	} else {
		response = "{\"code\": 400, \"response\": \"Bad PositionID data\"}";
	}
	return response;
}

/// @brief Sends a GET request with parameters
/// @param PositionID The positionID (vector index) of the webhook
/// @param parameters A map<String, String> of parameter names and values
/// @return A JSON string with "code" as the response code and "response" as the response payload
String WebhookManager::fireGet(int PositionID, std::map<String,String> Parameters) {
	String response;
	if (PositionID > 0 && PositionID < webhooks.size()) {
		response = webhooks[PositionID].hook->getRequest(Parameters);
	} else {
		response = "{\"code\": 400, \"response\": \"Bad PositionID data\"}";
	}
	return response;
}

/// @brief Sends a POST request with parameters in the format of a JSON encoded string
/// @param PositionID The positionID (vector index) of the webhook
/// @param parameters The JSON string to send
/// @return A JSON string with "code" as the response code and "response" as the response payload 
String WebhookManager::firePost(int PositionID, String Parameters) {
	String response;
	if (PositionID > 0 && PositionID < webhooks.size()) {
		response = webhooks[PositionID].hook->postRequest(Parameters);
	} else {
		response = "{\"code\": 400, \"response\": \"Bad PositionID data\"}";
	}
	return response;
}

/// @brief Sends a POST request with parameters in the format of a URL encoded string
/// @param PositionID The positionID (vector index) of the webhook
/// @param parameters A map of the names and values of the parameters
/// @return A JSON string with "code" as the response code and "response" as the response payload
String WebhookManager::firePost(int PositionID, std::map<String,String> Parameters) {
	String response;
	if (PositionID > 0 && PositionID < webhooks.size()) {
		response = webhooks[PositionID].hook->postRequest(Parameters);
	} else {
		response = "{\"code\": 400, \"response\": \"Bad PositionID data\"}";
	}
	return response;
}

/// @brief Converts the current webhooks to a JSON string
/// @return The webhooks as a JSON string
String WebhookManager::hooksToJSON() {
	// Allocate the JSON document
	JsonDocument doc;
	JsonArray hooks = doc["hooks"].to<JsonArray>();
	// Add webhook description
	for (const auto &h : webhooks) {
		hooks[h.positionID]["positionID"] = h.positionID;
		hooks[h.positionID]["url"] = h.hook->Description.url;
		// Add custom headers
		for (auto const &header : h.hook->Description.custom_headers) {
			hooks[h.positionID]["headers"][header.first] = header.second;
		}
	}
	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}