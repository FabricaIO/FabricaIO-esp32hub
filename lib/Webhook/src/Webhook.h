/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>

/// @brief Defines a generic webhook class for inheriting
class Webhook {
	public:		
		/// @brief Describes a webhook
		struct {
			/// @brief The full URL endpoint of this webhook
			String url;

			/// @brief A collection of custom header names and values
			std::map<String, String> custom_headers;
		} Description;

		Webhook(String url, std::map<String, String> customHeaders = {});
		String getRequest();
		String getRequest(String parameters);
		String getRequest(std::map<String, String> parameters);
		String postRequest(std::map<String, String> parameters);
		String postRequest(String parameters);
	
	private:
		/// @brief HTTPClient object
		HTTPClient client;

		/// @brief The format of the parameters for a POST request
		enum contentType { JSON, urlencoded };

		String sendGetRequest(String url_params);
		String sendPostRequest(std::map<String, String> parameters, contentType format);
		String parseParameters(std::map<String, String> parameters);
};
