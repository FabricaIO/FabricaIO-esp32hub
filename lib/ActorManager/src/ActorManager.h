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
		static QueueHandle_t actorQueue;

		/// @brief Holds all payloads delivered with a action
		static std::queue<String> payloads;

		static int actorNameToID(String name);
		static int actionNameToID(String name, int actorPosID);

	public:
		static bool addActor(Actor* actor);
		static bool beginActors();
		static bool addActionToQueue(String actor, String action, String payload = "");
		static bool addActionToQueue(int actorPosID, String action, String payload = "");
		static bool addActionToQueue(String actor, int actionID, String payload = "");
		static bool addActionToQueue(int actorPosID, int actionID, String payload = "");
		static std::tuple<bool, String> processActionImmediately(String actor, String action, String payload = "");
		static std::tuple<bool, String> processActionImmediately(int actorPosID, String action, String payload = "");
		static std::tuple<bool, String> processActionImmediately(String actor, int actionID, String payload = "");
		static std::tuple<bool, String> processActionImmediately(int actorPosID, int actionID, String payload = "");
		static String getActorInfo();
		static std::vector<Actor*> getActors();
		static String getActorConfig(int actorPosID);
		static bool setActorConfig(int actorPosID, String config);
		static String getActorVersions();
		static void actionProcessor(void* arg);
};