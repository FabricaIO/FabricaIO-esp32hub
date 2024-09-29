#include "ActorManager.h"

// Initialize static variables
std::vector<Actor*> ActorManager::actors;
QueueHandle_t ActorManager::actorQueue = xQueueCreate(15, sizeof(int[2]));
std::queue<String> ActorManager::payloads;

/// @brief Adds an actor to the in-use list
/// @param actor A pointer to the actor to add
/// @return True on success
bool ActorManager::addActor(Actor* actor) {
	// Add receiver to in-use list
	actors.push_back(actor);
	return true; // Currently no way to fail this
}

/// @brief Calls the begin function on all the in-use actors
/// @return True if all actors started correctly
bool ActorManager::beginActors() {
	for (auto const &a : actors) {
		if (!a->begin()) {
			Serial.println("Could not start " + a->Description.name);
			return false;
		} else {
			Serial.println("Started " + a->Description.name);
		}
	}
	return true;
}

/// @brief Adds an action to the queue for processing
/// @param actorPosID The position ID of the actor
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(int actorPosID, String action, String payload) {
	// Check if receiver is in-use
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Serial.println("Receiver position Id out of range");
		return false;
	}

	// Attempt to convert action name to ID
	int action_id;
	try {
		action_id = actors[actorPosID]->Description.actions.at("actor");
	} catch (const std::out_of_range& e) {
		Serial.println("Actor cannot process action");
		return false;
	}

	// Attempt to add action to queue
	return addActionToQueue(actorPosID, action_id, payload);
}

/// @brief Adds an action to the queue for processing
/// @param actorPosID The position ID of the actor
/// @param action The ID of the actor
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(int actorPosID, int action, String payload) {
	// Check if receiver is in-use
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Serial.println("Receiver position ID out of range");
		return false;
	}

	// Create action array for queue
	int new_action[] { actorPosID, action };

	// Add actor array to queue
	if (xQueueSend(actorQueue, &new_action, 10) != pdTRUE) {
		Serial.println("Actor queue full");
		return false;
	}
	// Add payload to queue
	payloads.push(payload);
	return true;
}

/// @brief Retrieves the information on all available actors and their actors
/// @return A JSON string of the information
String ActorManager::getActorInfo() {
	// Allocate the JSON document
	JsonDocument doc;
	// Create array of actors
	JsonArray receiver_array = doc["actors"].to<JsonArray>();

	for (int i = 0; i < actors.size(); i++) {
		// Add receiver description to JSON document 
		receiver_array[i]["positionID"] = i;
		receiver_array[i]["description"]["actorQuantity"] = actors[i]->Description.actionQuantity;
		receiver_array[i]["description"]["type"] = actors[i]->Description.type;
		receiver_array[i]["description"]["name"] = actors[i]->Description.name;
		receiver_array[i]["description"]["id"] = actors[i]->Description.id;
		// Add actors and IDs to JSON document
		for (auto const &s : actors[i]->Description.actions) {
			receiver_array[i]["actions"][s.second] = s.first;
		}
	}
	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}


/// @brief Gets any available config settings for a actor receive device
/// @param actorPosID The position ID of the actor receive
/// @return A JSON string of configurable settings
String ActorManager::getActorConfig(int actorPosID) {
	if (actorPosID >= 0 && actorPosID < actors.size()) {
		return actors[actorPosID]->getConfig();
	} else {
		return "{}";
	}
}

/// @brief Gets any available config settings for a actor receiver device
/// @param actorPosID The position ID of the actor receive
/// @param config A JSON string of the configuration
/// @return True on success
bool ActorManager::setActorConfig(int actorPosID, String config) {
	if (actorPosID >= 0 && actorPosID < actors.size()) {
		return actors[actorPosID]->setConfig(config);
	} else {
		return false;
	}
}

/// @brief Gets all the current actor versions
/// @return A JSON string of all sensor versions
String ActorManager::getActorVersions() {
	String output;
	if (actors.size() > 0) {
		// Allocate the JSON document
		JsonDocument doc;
		// Add versions to object
		for (const auto& a : actors) {
			doc[a->Description.name] = a->Description.version;
		}	
		serializeJson(doc, output);
	} else {
		output = "{}";
	}
	return output;
}

/// @brief Executes a actor on a receiver immediately. Use carefully, may cause issues with actors also being processed from queue
/// @param actorPosID The position ID of the actor receiver
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(int actorPosID, String action, String payload) {
	// Check if receiver is in-use
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Serial.println("Receiver position Id out of range");
		return { true, R"({"success": false})" };
	}

	// Attempt to convert actor name to ID
	int actor_id;
	try {
		actor_id = actors[actorPosID]->Description.actions.at(action);
	} catch (const std::out_of_range& e) {
		Serial.println("Receiver cannot process actor");
		return{ true, R"({"success": false})" };
	}
	// Process actor
	return processActionImmediately(actorPosID, actor_id, payload);
}

/// @brief Executes a action on a actor immediately. Use carefully, may cause issues with actions also being processed from queue
/// @param actorPosID The position ID of the actor receiver
/// @param action The ID of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(int actorPosID, int action, String payload) {
	// Check if receiver is in-use
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Serial.println("Receiver position ID out of range");
		return { true, R"({"success": false})" };
	}
	// Process actor
	return actors[actorPosID]->receiveAction(action, payload);
}

/// @brief Wraps the actor processor task for static access
/// @param arg Not used
void ActorManager::actionProcessor(void* arg) {
	int action[2];
	while(true) {
		if (xQueueReceive(actorQueue, &action, 10) == pdTRUE)
		{
			actors[action[0]]->receiveAction(action[1], payloads.front());
			payloads.pop();
		}
		delay(100);
	}
}