#include "Webhook.h"

/// @brief Creates a new Webhook
/// @param URL The URL endpoint of the webhook
/// @param URcustomHeadersL Optional custom headers as name and value pairs
Webhook::Webhook(String URL, std::map<String, String> customHeaders) {
	Description.url = URL;
	Description.custom_headers = customHeaders;
}

/// @brief Sends a GET request with no parameters
/// @return A JSON string with "code" as the response code and "response" as the response payload
String Webhook::getRequest() {
	return sendGetRequest("");
}

/// @brief Sends a GET request with parameters
/// @param parameters A map<String, String> of parameter names and values
/// @return A JSON string with "code" as the response code and "response" as the response payload
String Webhook::getRequest(std::map<String, String> parameters) {
	return sendGetRequest("?" + parseParameters(parameters));
}

/// @brief Sends a GET request with parameters
/// @param parameters A JSON string of parameter names and values
/// @return A JSON string with "code" as the response code and "response" as the response payload
String Webhook::getRequest(String parameters) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, parameters);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		return "{\"code\": 400, \"response\": \"Bad JSON data\"}";
	}
	// Build parameter map
	std::map<String, String> params;
	for (JsonPair param : doc.as<JsonObject>()) {
		params[param.key().c_str()] = param.value().as<String>();
	}
	return sendGetRequest("?" + parseParameters(params));
}

/// @brief Sends a POST request with parameters in the format of a URL encoded string
/// @param parameters A map of the names and values of the parameters
/// @return A JSON string with "code" as the response code and "response" as the response payload
String Webhook::postRequest(std::map<String, String> parameters) {
	return sendPostRequest(parameters, contentType::urlencoded);
}

/// @brief Sends a POST request with parameters in the format of a JSON encoded string
/// @param parameters The JSON string to send
/// @return A JSON string with "code" as the response code and "response" as the response payload 
String Webhook::postRequest(String parameters) {
	return sendPostRequest(std::map<String, String>{{"params", parameters}}, contentType::JSON);
}

/// @brief Sends a GET request
/// @param url_params String representing the URL encoded GET parameters, if any
/// @return A JSON string with "code" as the response code and "response" as the response payload
String Webhook::sendGetRequest(String url_params) {
	client.begin(Description.url + url_params);
	// Add any custom headers
	if (!Description.custom_headers.empty()) {
		for (std::pair<String, String> header : Description.custom_headers) {
			client.addHeader(header.first, header.second);
		}
	}
	// Get response
	int response_code = client.GET();
	String result = "{\"code\": " + String(response_code) + ",\"response\": \"";
	if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_ACCEPTED) {
		String response = client.getString();
		Serial.println(response);
		result += response + "\"}";
	} else {
		Serial.print("Webhook failed. Response code: ");
		Serial.println(response_code);
		result += "fail\"}";
	}
	client.end();
	return result;
}

/// @brief Sends a POST request
/// @param parameters The POST parameters
/// @param format The format of the POST parameters
/// @return A JSON string with "code" as the response code and "response" as the response payload 
String Webhook::sendPostRequest(std::map<String, String> parameters, contentType format) {
	client.begin(Description.url);
	String params;
	// Add any custom headers
	if (!Description.custom_headers.empty()) {
		for (std::pair<String, String> header : Description.custom_headers) {
			client.addHeader(header.first, header.second);
		}
	}
	if (format == contentType::JSON) {
		client.addHeader("Content-Type", "text/json");
		params = parameters["params"];
	} else {
		params = parseParameters(parameters);
		client.addHeader("Content-Type", "application/x-www-form-urlencoded");
	}
	// Get response
	int response_code = client.POST(params);
	String result = "{\"code\": " + String(response_code) + ",\"response\": \"";
	if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_ACCEPTED) {
		String response = client.getString();
		Serial.println(response);
		result += response + "\"}";
	} else {
		Serial.print("Webhook failed. Response code: ");
		Serial.println(response_code);
		result += "fail\"}";
	}
	client.end();
	return result;
}

/// @brief Parses a map of parameter names and values to a query string (param1=foo&param2=bar etc...)
/// @param parameters  A map<String, String> of parameter names and values
/// @return The formatted query string
String Webhook::parseParameters(std::map<String, String> parameters) {
	String params = "";
	if (!parameters.empty()) {
		bool first = true;
		for (std::pair<String, String> param : parameters) {
			if (first) {
				first = false;
			} else {
				params += '&';
			}
			params += param.first + '=' + param.second;
		}
	}
	return params;
}