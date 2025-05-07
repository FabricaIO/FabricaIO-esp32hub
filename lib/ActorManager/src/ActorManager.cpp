#include "ActorManager.h"

// Initialize static variables
std::vector<Actor*> ActorManager::actors;
QueueHandle_t ActorManager::actorQueue = xQueueCreate(15, sizeof(int[2]));
QueueHandle_t ActorManager::payloads = xQueueCreate(15, sizeof(String*));

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
			Logger.println("Could not start " + a->Description.name);
			return false;
		} else {
			Logger.println("Started " + a->Description.name);
		}
	}
	return true;
}

/// @brief Adds an action to the queue for processing
/// @param actor The name of the actor
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(String actor, String action, String payload) {
	// Attempt to convert actor name to ID
	int actorPosID = actorNameToID(actor);
	if (actorPosID == -1) {
		return false;
	}

	// Attempt to convert action name to ID
	int action_id = actionNameToID(action, actorPosID);
	if (action_id == -1) {
		return false;
	}

	// Attempt to add action to queue
	return addActionToQueue(actorPosID, action_id, payload);
}

/// @brief Adds an action to the queue for processing
/// @param actorPosID The position ID of the actor
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(int actorPosID, String action, String payload) {
	// Check if actor exists
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Logger.println("Actor position ID out of range");
		return false;
	}

	// Attempt to convert action name to ID
	int action_id = actionNameToID(action, actorPosID);
	if (action_id == -1) {
		return false;
	}

	// Attempt to add action to queue
	return addActionToQueue(actorPosID, action_id, payload);
}

/// @brief Adds an action to the queue for processing
/// @param actor The name of the actor
/// @param actionID The ID of the action
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(String actor, int actionID, String payload) {
	// Attempt to convert actor name to ID
	int actorPosID = actorNameToID(actor);
	if (actorPosID == -1) {
		return false;
	}

	// Attempt to add action to queue
	return addActionToQueue(actorPosID, actionID, payload);
}

/// @brief Adds an action to the queue for processing
/// @param actorPosID The position ID of the actor
/// @param actionID The ID of the action
/// @param payload An optional JSON string for data payload
/// @return True on success
bool ActorManager::addActionToQueue(int actorPosID, int actionID, String payload) {
	// Check if actor exists
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Logger.println("Actor position ID out of range");
		return false;
	}

	// Create action array for queue
	int new_action[] { actorPosID, actionID };

	// Add actor array to queue
	if (xQueueSend(actorQueue, &new_action, 10) != pdTRUE) {
		Logger.println("Actor queue full");
		return false;
	}
	// Add payload to queue
	String* payload_ptr = new String(payload);
	if (xQueueSend(payloads, &payload_ptr, 100 / portTICK_PERIOD_MS) != pdTRUE) {
		delete payload_ptr;
		Logger.println("Payload queue full");
		return false;
	}
	return true;
}

/// @brief Retrieves the information on all available actors and their actions
/// @return A JSON string of the information
String ActorManager::getActorInfo() {
	// Allocate the JSON document
	JsonDocument doc;
	// Create array of actors
	JsonArray actor_array = doc["actors"].to<JsonArray>();

	for (int i = 0; i < actors.size(); i++) {
		// Add receiver description to JSON document 
		actor_array[i]["positionID"] = i;
		actor_array[i]["description"]["actionQuantity"] = actors[i]->Description.actionQuantity;
		actor_array[i]["description"]["type"] = actors[i]->Description.type;
		actor_array[i]["description"]["name"] = actors[i]->Description.name;
		actor_array[i]["description"]["version"] = actors[i]->Description.version;
		// Add actors and IDs to JSON document
		for (auto const &a : actors[i]->Description.actions) {
			actor_array[i]["actions"][a.second] = a.first;
		}
	}
	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Gets all actors
/// @return A vector with pointers to each actor
std::vector<Actor*> ActorManager::getActors() {
	return actors;
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
		return actors[actorPosID]->setConfig(config, true);
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
/// @param actor The position ID of the actor receiver
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(String actor, String action, String payload) {
	// Try to convert actor to ID
	int actorPosID = actorNameToID(actor);
	if (actorPosID == -1) {
		return{ true, R"({"success": false})" };
	}
	
	// Attempt to convert actor name to ID
	int action_id = actionNameToID(action, actorPosID);
	if (action_id == -1) {
		return{ true, R"({"success": false})" };
	}
	// Process action
	return processActionImmediately(actorPosID, action_id, payload);
}

/// @brief Executes a actor on a receiver immediately. Use carefully, may cause issues with actors also being processed from queue
/// @param actorPosID The position ID of the actor receiver
/// @param action The name of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(int actorPosID, String action, String payload) {
	// Check if actor exists
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Logger.println("Receiver position Id out of range");
		return { true, R"({"success": false})" };
	}

	// Attempt to convert actor name to ID
	int action_id = actionNameToID(action, actorPosID);
	if (action_id == -1) {
		return{ true, R"({"success": false})" };
	}

	// Process action
	return processActionImmediately(actorPosID, action_id, payload);
}

/// @brief Executes a action on a actor immediately. Use carefully, may cause issues with actions also being processed from queue
/// @param actor The position ID of the actor receiver
/// @param actionID The ID of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(String actor, int actionID, String payload) {
	// Try to convert actor to ID
	int actorPosID = actorNameToID(actor);
	if (actorPosID == -1) {
		return{ true, R"({"success": false})" };
	}
	
	// Process action
	return processActionImmediately(actorPosID, actionID, payload);
}

/// @brief Executes a action on a actor immediately. Use carefully, may cause issues with actions also being processed from queue
/// @param actorPosID The position ID of the actor receiver
/// @param actionID The ID of the action
/// @param payload An optional JSON string for data payload
/// @return A tuple with a string containing any response, and a bool indicating if it's JSON formatted
std::tuple<bool, String> ActorManager::processActionImmediately(int actorPosID, int actionID, String payload) {
	// Check if actor exists
	if(actorPosID < 0 || actorPosID >= actors.size()) {
		Logger.println("Receiver position ID out of range");
		return { true, R"({"success": false})" };
	}
	// Process action
	return actors[actorPosID]->receiveAction(actionID, payload);
}

/// @brief Turns the name of an actor into its position ID
/// @param name Then name of the actor
/// @return The positionID of the actor or -1 on failure
int ActorManager::actorNameToID(String name) {
	int actorPosID;
	try {
		auto index = std::find_if(actors.begin(), actors.end(), [name](Actor* a) {return a->Description.name == name;});
		if (index == actors.end()) {
			Logger.println("Actor not found");
			return -1;
		}
		actorPosID = index - actors.begin();
	} catch (const std::exception& e) {
		Logger.println("Actor not found");
		return -1;
	}
	return actorPosID;
}

/// @brief Turns the name of an action into its  ID
/// @param name Then name of the action
/// @param actorPosID The ID of the of actor the action belongs to
/// @return The ID of the action or -1 on failure
int ActorManager::actionNameToID(String name, int actorPosID) {
	int actor_id;
	try {
		actor_id = actors[actorPosID]->Description.actions.at(name);
	} catch (const std::out_of_range& e) {
		Logger.println("Receiver cannot process actor");
		return -1;
	}
	return actor_id;
}

/// @brief Wraps the actor processor task for static access
/// @param arg Not used
void ActorManager::actionProcessor(void* arg) {
	int action[2];
	String* payload;
	while(true) {
		if (xQueueReceive(actorQueue, &action, 10) == pdTRUE)
		{
			if (xQueueReceive(payloads, &payload, 100 / portTICK_PERIOD_MS) == pdTRUE)
			{
				try {
					actors[action[0]]->receiveAction(action[1], *payload);
					delete payload;
				}
				catch (...) {
					delete payload;
					Logger.println("Exception in processing action payload from queue");
				}
			}
		}
		delay(100);
	}
}