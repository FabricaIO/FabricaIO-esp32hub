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
#include <Webhook.h>
#include <Storage.h>
#include <vector>
#include <map>

/// @brief Manages all available webhooks
class WebhookManager {
	private:
		typedef struct Webhook_info {
			/// @brief Ths positional ID of the webhook (where it's located in the vector)
			int positionID;

			/// @brief A pointer to the webhook object
			std::shared_ptr<Webhook> hook;
		} Webhook_info;

		/// @brief Stores all the in-use webhooks
		static std::vector<Webhook_info> webhooks;

		/// @brief Full path to config file
		static String config;
		
		static String hooksToJSON();

	public:
		static bool begin(String configFile);
		static bool loadWebhooks();
		static bool saveWebhooks();
		static bool saveWebhooks(String hooks);
		static bool updateWebhooks(String hooks);
		static String getWebhooks();
		static String fireGet(int positionID);
		static String fireGet(int positionID, String parameters);
		static String fireGet(int positionID, std::map<String,String> parameters);
		static String firePost(int positionID, String parameters);
		static String firePost(int positionID, std::map<String,String> parameters);
};