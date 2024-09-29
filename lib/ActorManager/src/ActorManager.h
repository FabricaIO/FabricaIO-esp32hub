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

	public:
		static bool addActor(Actor* actor);
		static bool beginActors();
		static bool addActionToQueue(int actorPosID, String actor, String payload = "");
		static bool addActionToQueue(int actorPosID, int actor, String payload = "");
		static std::tuple<bool, String> processActionImmediately(int actorPosID, String actor, String payload = "");
		static std::tuple<bool, String> processActionImmediately(int actorPosID, int actor, String payload = "");
		static String getActorInfo();
		static String getActorConfig(int actorPosID);
		static bool setActorConfig(int actorPosID, String config);
		static String getActorVersions();
		static void actionProcessor(void* arg);
};