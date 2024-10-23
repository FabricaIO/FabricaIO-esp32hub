#include "Webserver.h"

/// @brief Holds current firmware version
extern const String FW_VERSION;

/// @brief Indicates if the hub booted successfully
extern bool POSTSuccess;

// Initialize static variables
bool Webserver::upload_abort = false;
bool Webserver::shouldReboot = false;
int Webserver::upload_response_code = 201;

/// @brief Creates a Webserver object
/// @param Webserver A pointer to an AsyncWebServer object
Webserver::Webserver(AsyncWebServer* Webserver) {
	server = Webserver;
}

/// @brief Starts the update server
bool Webserver::ServerStart() {
	Logger.println("Starting web server");

	// Create root directory if needed
	if (!Storage::fileExists("/www"))
		if (!Storage::createDir("/www"))
			return false;

	// Add request handler for index page
	if (Storage::fileExists("/www/index.html")) {
		// Serve any page from filesystem
		server->serveStatic("/", *Storage::getFileSystem(), "/www/").setDefaultFile("index.html").setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());
	} else {
		// Serve the embedded index page
		server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
			request->send_P(HTTP_CODE_OK, "text/html", index_page);
		}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());
	}

	// Handle file uploads
	server->on("/upload-file", HTTP_POST, [](AsyncWebServerRequest *request) {
		// Let upload start
		delay(50);
		// Construct response
		AsyncWebServerResponse *response = request->beginResponse(Webserver::upload_response_code, "text/plain", Webserver::upload_abort ? "Upload failed": "File uploaded");
		response->addHeader("Connection", "close");
		request->send(response);
	}, onUpload_file).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle deletion of files
	server->on("/delete", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if(request->hasParam("path", true)) {
			String path = request->getParam("path", true)->value();
			Logger.println("Deleting " + path);
			if (Storage::fileExists(path)) {
				if (!Storage::deleteFile(path)) {
					request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not delete file");
				} else {
					request->send(HTTP_CODE_OK, "text/json", "{\"file\":\"" + path + "\"}");
				}
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Get descriptions of available sensors
	server->on("/sensors/", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", SensorManager::getSensorInfo());
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Get curent configuration of a sensor
	server->on("/sensors/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("sensor")) {
			int sensorPosID = request->getParam("sensor")->value().toInt();
			request->send(HTTP_CODE_OK, "text/json", SensorManager::getSensorConfig(sensorPosID));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Update configuration of a sensor
	server->on("/sensors/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("sensor", true)) {
			// Parse data payload
			int sensorPosID = request->getParam("sensor", true)->value().toInt();
			String config = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (SensorManager::setSensorConfig(sensorPosID, config)) {
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());
	// Gets last measurement. Add GET paramater "update" (/sensors/measurement?update) to take a new measurement first
	server->on("/sensors/measurement", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (POSTSuccess) {
			if (request->hasParam("update")) {
				// Attempt to take new measurement
				if (!SensorManager::takeMeasurement()) {
					request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not take measurement");
					return;
				}
			}
			request->send(HTTP_CODE_OK, "text/json", SensorManager::getLastMeasurement());
		} else {
			request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());
	
	// Runs a calibration procedure on a sensor
	server->on("/sensors/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (POSTSuccess) {
			if (request->hasParam("sensor", true) && request->hasParam("step", true)) {
				// Parse data payload
				int sensorPosID = request->getParam("sensor", true)->value().toInt();
				int step = request->getParam("step", true)->value().toInt();

				// Run sensor calibration
				std::tuple<Sensor::calibration_response, String> response = SensorManager::calibrateSensor(sensorPosID, step);

				// Create response
				request->send(HTTP_CODE_OK, "text/json", "{\"response\":" + String(std::get<0>(response)) + ",\"message\":\"" + std::get<1>(response) + "\"}");
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
			}
		} else {
			request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Get descriptions of available actors
	server->on("/actors/", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", ActorManager::getActorInfo());
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Get curent configuration of a receiver
	server->on("/actors/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("actor")) {
			int actorPosID = request->getParam("actor")->value().toInt();
			request->send(HTTP_CODE_OK, "text/json", ActorManager::getActorConfig(actorPosID));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Update configuration of an actor
	server->on("/actors/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("actor", true)) {
			// Parse data payload
			int actorPosID = request->getParam("actor", true)->value().toInt();
			String config = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (ActorManager::setActorConfig(actorPosID, config)) {
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Adds an action to the action queue using the action's name or ID
	server->on("/actors/add", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (POSTSuccess) {
			if (request->hasParam("actor", true) && (request->hasParam("id", true) || request->hasParam("name", true))) {
				// Parse data payload
				bool id = true;
				int actorPosID = request->getParam("actor", true)->value().toInt();
				String payload = "";
				if (request->hasParam("payload", true)) {
					payload = request->getParam("payload", true)->value();
				}
				// Attempt to add signal to queue
				bool success = false;
				if (request->hasParam("id", true)) {
					success = ActorManager::addActionToQueue(actorPosID, request->getParam("id", true)->value().toInt(), payload);
				} else {
					success = ActorManager::addActionToQueue(actorPosID, request->getParam("name", true)->value(), payload);
				}
				if (!success) {
					request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not add signal to queue");
				} else {
					request->send(HTTP_CODE_OK, "text/plain", "OK");
				}
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
			}
		} else {
			request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Sends a action to an actor immediately using the action's name or ID, and returns any response
	server->on("/actors/execute", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (POSTSuccess){
			if (request->hasParam("actor") && (request->hasParam("id") || request->hasParam("name"))) {
				// Parse data payload
				bool id = true;
				int actorPosID = request->getParam("actor")->value().toInt();
				String payload = "";
				if (request->hasParam("payload")) {
					payload = request->getParam("payload")->value();
				}
				std::tuple<bool, String> result;
				if (request->hasParam("id")) {
					result = ActorManager::processActionImmediately(actorPosID, request->getParam("id")->value().toInt(), payload);
				} else {
					result = ActorManager::processActionImmediately(actorPosID, request->getParam("name")->value(), payload);
				}
				String mime = "text/json";
				if (!std::get<0>(result)) {
					mime = "text/plain";
				}
				// Execute signal and return response
				request->send(HTTP_CODE_OK, mime, std::get<1>(result));
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
			}
		} else {
			request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Sends an action to a actor immediately using the action'a name or ID, and returns any response
	server->on("/actors/execute", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (POSTSuccess) {	
			if (request->hasParam("actor", true) && (request->hasParam("id", true) || request->hasParam("name", true))) {
				// Parse data payload
				bool id = true;
				int actorPosID = request->getParam("actor", true)->value().toInt();
				String payload = "";
				if (request->hasParam("payload", true)) {
					payload = request->getParam("payload", true)->value();
				}
				std::tuple<bool, String> result;
				if (request->hasParam("id", true)) {
					result = ActorManager::processActionImmediately(actorPosID, request->getParam("id", true)->value().toInt(), payload);
				} else {
					result = ActorManager::processActionImmediately(actorPosID, request->getParam("name", true)->value(), payload);
				}
				String mime = "text/json";
				if (!std::get<0>(result)) {
					mime = "text/plain";
				}
				// Execute signal and return response
				request->send(HTTP_CODE_OK, mime, std::get<1>(result));
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
			}
		} else {
			request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Get curent global configuration
	server->on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", Configuration::getConfig());
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Update global configuration
	server->on("/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("save", true)) {
			// Parse data payload
			bool save = request->getParam("save", true)->value() == "true";
			String config_string = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (Configuration::updateConfig(config_string)) {
				if (save) {
					// Attempt to save config
					if(!Configuration::saveConfig(config_string)) {
						request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not save config settings");
						return;
					}
				}
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Gets the time on the device
	server->on("/time", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/plain", String(TimeInterface::getLocalEpoch()));
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Sets the time on the device
	server->on("/time", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("time", true)) {
			// Parse data payload
			long time = request->getParam("time", true)->value().toInt();
			
			// Apply time settings
			TimeInterface::setTime(time);

			Logger.print("Set time to: ");
			Logger.println(TimeInterface::getDateTime());
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle request for the amount of free space on the storage device (example of returning JSON data)
	server->on("/freeSpace", HTTP_GET, [this](AsyncWebServerRequest *request) {	
		String result = "{ \"space\": " + String(Storage::freeSpace()) + " }";
		request->send(HTTP_CODE_OK, "text/json", result);
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle reset request
	server->on("/reset", HTTP_PUT, [this](AsyncWebServerRequest *request) {
		Logger.println("Resetting WiFi settings");
		 if (Storage::fileExists("/www/reset.html")) {
			request->send(*Storage::getFileSystem(), "/www/reset.html", "text/html");
		} else {
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		}
		WiFi.mode(WIFI_AP_STA); // Cannot erase if not in STA mode !
		WiFi.persistent(true);
		WiFi.disconnect(true, true);
		WiFi.persistent(false);
		Webserver::shouldReboot = true;
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle reboot request
	server->on("/reboot", HTTP_PUT, [this](AsyncWebServerRequest *request) {
		if (Storage::fileExists("/www/reboot.html")) {
			request->send(*Storage::getFileSystem(), "/www/reboot.html", "text/html");
		} else {
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		}
		Webserver::shouldReboot = true;
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle listing files and directories
	server->on("/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("path")) {
			String path = request->getParam("path")->value();
			if (Storage::fileExists(path)) {
				int depth = 0;
				if (request->hasParam("depth")) {
					depth = request->getParam("depth")->value().toInt();
				}
				int type = 0;
				if (request->hasParam("type")) {
					type =  request->getParam("type")->value().toInt();
				}
				std::vector<String> list;
				if (type == 0) {
					list = Storage::listFiles(path, depth);
				} else {
					list = Storage::listDirs(path, depth);
				}
				JsonDocument result;
				for (int i = 0; i < list.size(); i++) {
					result["list"][i] = list[i];
				}
				String result_string;
				serializeJson(result, result_string);
				request->send(HTTP_CODE_OK, "text/json", result_string);
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Folder doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Handle downloads
	server->on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("path")) {
			String path = request->getParam("path")->value();
			if (Storage::fileExists(path)) {
				request->send(*Storage::getFileSystem(), path, "application/octet-stream");
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Allow files to be restored by string input
	server->on("/restorefile", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("path", true) && request->hasParam("contents", true)) {
			// Change to Unix line endings to save space
			String content = request->getParam("contents", true)->value();
			content.replace("\r\n", "\n");
			// Check for, and create, directories
			size_t pos = 0;
			String path_builder = "";
			// Remove starting '/'
			String path = request->getParam("path", true)->value();
			path.remove(0,1);
			while ((pos = path.indexOf('/')) != -1) {
				path_builder += "/" + path.substring(0, pos);
				if (!Storage::fileExists(path_builder)) {
					Storage::createDir(path_builder);
				}
				path.remove(0, pos + 1);
			}
			if(Storage::writeFile(request->getParam("path", true)->value(), content)) {
				request->send(HTTP_CODE_OK, "text/plain", "File restored");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not restore file");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Used to fetch current firmware versions
	server->on("/version", HTTP_GET, [this](AsyncWebServerRequest *request) {
		String versions = "{\"hub\":\"" + FW_VERSION + "\",";
		versions += "\"logreceivers\":" + Logger.getReceiverVersions() + ",";
		versions += "\"eventreceivers\":" + EventBroadcaster::getReceiverVersions() + ",";
		versions += "\"sensors\":" + SensorManager::getSensorVersions() + ",";
		versions += "\"actors\":" + ActorManager::getActorVersions();
		versions += "}";
		request->send(HTTP_CODE_OK, "text/json", versions);
	});

	// Update page is special and hard-coded to always be available
	server->on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send_P(HTTP_CODE_OK, "text/html", update_page);
	}).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());

	// Update firmware
	server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
		// Let update start
		delay(50);
		
		// Check if should reboot
		Webserver::shouldReboot = !Update.hasError();

		// Construct response
		AsyncWebServerResponse *response = request->beginResponse(Webserver::shouldReboot ? HTTP_CODE_ACCEPTED : HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", this->Webserver::shouldReboot ? "OK" : "ERROR");
		response->addHeader("Connection", "close");
		request->send(response);
	}, onUpdate).setAuthentication(Configuration::currentConfig.webUsername.c_str(), Configuration::currentConfig.webPassword.c_str());    

	// 404 handler
	server->onNotFound([](AsyncWebServerRequest *request) { 
		request->send(HTTP_CODE_NOT_FOUND); 
	});

	server->begin();
	return true;
}

/// @brief Stops the update server
void Webserver::ServerStop() {
	Logger.println("Stopping web server");
	server->reset();
	server->end();
}

/// @brief Wraps the reboot checker task for static access
/// @param arg The Webserver object
void Webserver::RebootCheckerTaskWrapper(void* arg) {
	static_cast<Webserver*>(arg)->RebootChecker();
}

/// @brief Checks if a reboot was requested
void Webserver::RebootChecker() {
	while (true) {
		if (Webserver::shouldReboot) {
			Logger.println("Rebooting from API call...");
			// Pause automation before reboot
			Configuration::currentConfig.tasksEnabled = false;
			// Delay to show event messages, let server respond, and finish any automation
			EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Rebooting);
			delay(3000);
			ESP.restart();
		}
		// This loop doesn't need to be tight
		delay(500);
	}
}

/// @brief Handle file uploads to a folder. Adapted from https://github.com/smford/esp32-asyncwebserver-fileupload-example
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpload_file(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	if (!index) {
		if (!request->hasHeader("FILE_UPLOAD_PATH")) {
			final = true;
			Webserver::upload_abort = true;
			Webserver::upload_response_code = HTTP_CODE_BAD_REQUEST;
			return;
		}
		String path = request->header("FILE_UPLOAD_PATH");
		Webserver::upload_abort = false;
		request->_tempFile = Storage::getFileSystem()->open(path + "/" + filename, "w", true);
		Logger.println("Uploading file " + filename);
	}
	if (Webserver::upload_abort)
		return;
	if (len) {
		// Stream the incoming chunk to the opened file
		if (request->_tempFile.write(data, len) != len) {
			final = true;
			Webserver::upload_abort = true;
			Webserver::upload_response_code = HTTP_CODE_INSUFFICIENT_STORAGE;
		}
	}
	if (final) {
		// Close the file handle as the upload is now done
		String path = request->_tempFile.path();
		request->_tempFile.close();
		if (Webserver::upload_abort) {
			// Remove failed upload
			Storage::deleteFile(path);
		} else {
			Webserver::upload_response_code = HTTP_CODE_CREATED;
		}
	}
}

/// @brief Handle firmware update
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	if (!index)
	{
		Logger.printf("Update Start: %s\n", filename.c_str());
		// Pause automation during update
		Configuration::currentConfig.tasksEnabled = false;
		delay(100);
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Updating);
		// Ensure firmware will fit into flash space
		if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
		{
			Update.printError(Serial);
		}
	}
	if (!Update.hasError())
	{
		if (Update.write(data, len) != len)
		{
			Update.printError(Serial);
		}
	}
	if (final)
	{
		if (Update.end(true))
		{
			Logger.printf("Update Success: %uB\n", index + len);
		}
		else
		{
			Update.printError(Serial);
		}
	}
}