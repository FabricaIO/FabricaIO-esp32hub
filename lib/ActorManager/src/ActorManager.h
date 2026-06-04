/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <ArduinoJson.h>
#include <Actor.h>
#include <vector>
#include <queue>

/// @brief Receives and processes actions for actor
class ActorManager {
	private:
		/// @brief Stores all the in-use actor actors
		static std::vector<Actor*> actors;

		/// @brief Queue to hold action to be processed.
		static QueueHandle_t actionQueue;

		/// @brief Struct for grouping action and payloads
		struct actionCall {
			int actorPosID;
			int actionID;
			String* payload;
		};

	public:
		/// @brief Task handle for action processor loop
		static TaskHandle_t actionHandle;

		/// @brief Mutex for protecting access to task handle
		static SemaphoreHandle_t taskMutex;

		/// @brief True when there are no actor devices
		static bool noActors;
		
		static bool addActor(Actor* actor);
		static bool beginActors();
		static bool addActionToQueue(String actor, String action, String payload = "");
		static bool addActionToQueue(int actorPosID, String action, String payload = "");
		static bool addActionToQueue(String actor, int actionID, String payload = "");
		static bool addActionToQueue(int actorPosID, int actionID, String payload = "");
		static std::pair<bool, String> processActionImmediately(String actor, String action, String payload = "");
		static std::pair<bool, String> processActionImmediately(int actorPosID, String action, String payload = "");
		static std::pair<bool, String> processActionImmediately(String actor, int actionID, String payload = "");
		static std::pair<bool, String> processActionImmediately(int actorPosID, int actionID, String payload = "");
		static String getActorInfo();
		static std::vector<Actor*> getActors();
		static String getActorConfig(int actorPosID);
		static String getActorConfig(String actorName);
		static bool setActorConfig(int actorPosID, String config);
		static String getActorVersions();
		static void actionProcessor(void* arg);
		static int actorNameToID(String name);
		static int actionNameToID(String name, int actorPosID);
};