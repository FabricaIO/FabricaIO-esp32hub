/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ESPAsyncWebServer: https://github.com/esphome/ESPAsyncWebServer
 * ArduinoJSON: https://arduinojson.org/
 * ESP32Time: https://github.com/fbiego/ESP32Time
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <SD.h>
#include <TimeInterface.h>
#include <Storage.h>
#include <Configuration.h>
#include <SensorManager.h>
#include <ActorManager.h>
#include <HTTPClient.h>
#include <EventBroadcaster.h>
#include <LogBroadcaster.h>
#include <vector>

/// @brief Local web server.
class Webserver {
	public:		
		Webserver(AsyncWebServer* webserver);
		bool ServerStart();
		void ServerStop();
		static void RebootChecker(void* arg);
		
	private:
		/// @brief Pointer to the Webserver object
		AsyncWebServer* server;

		/// @brief Used to indicate an upload had to be aborted
		static bool upload_abort;

		/// @brief Used to indicate the status code of the last upload
		static int upload_response_code;

		/// @brief Used to signal that a reboot is requested or needed
		static bool shouldReboot;

		/// @brief Authentication middleware for auth
		static AsyncAuthenticationMiddleware authMiddleware;
		
		/// @brief CORS middleware
		AsyncCorsMiddleware corsMiddleware;

		/// @brief This middleware is needed to expose www-authenticate and related headers 
		class CORSAuthFixMiddleware : public AsyncMiddleware {
			public:
				void run(AsyncWebServerRequest *request, ArMiddlewareNext next) override {
					if (request->hasHeader(asyncsrv::T_CORS_O)) {
						// check if this is a preflight request => handle it and return
						next();					
						AsyncWebServerResponse *response = request->getResponse();
						if (response) {
							response->addHeader("Access-Control-Expose-Headers", "*");
						}
					} else {
						next();
					}
				}
			};
				
		/// @brief CORS middleware fix
		CORSAuthFixMiddleware corsMiddlewareFix;

		static void onUpload_file(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
		static void onUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
};

// @brief Text of update webpage
const char update_page[] = R"(<!DOCTYPE html>
<html lang='en-us'>
<head>
<title>Firmware Updater</title>
</head>
<body>
<div id='up-wrap'>
<div class='home-button-container'>
	<a class='def-button' href="/">Home</a>
</div>
<h1>Firmware Updater</h1>
<h2>Upload Firmware</h2>
<h3 id='fw'>Current version: </h3>
<div id='up-progress'>
	<div id='up-bar'></div>
	<div id='up-percent'>0%</div>
</div>
<input type='file' id='up-file' disabled>
<label for='up-file' class='def-button' id='up-label'>
	Update
</label>
<div id='message'></div>
</div>
<script>
let uprog = {
	hBar: null,
	hPercent: null,
	hFile: null,
	init: () => {
		uprog.hBar = document.getElementById('up-bar');
		uprog.hPercent = document.getElementById('up-percent');
		uprog.hFile = document.getElementById('up-file');
		uprog.hFile.disabled = false;
	},
	update: (percent) => {
		uprog.hBar.style.width = percent + '%';
		uprog.hPercent.innerHTML = percent + '%';
		if (percent === 100) uprog.hFile.disabled = false;
	},
	upload: (path) => {
		return new Promise((resolve, reject) => {
			if (uprog.hFile.files.length == 0) {
				reject(new Error('No file selected'));
				return;
			}
			
			let file = uprog.hFile.files[0];
			uprog.hFile.disabled = true;
			uprog.hFile.value = '';
			let xhr = new XMLHttpRequest(), data = new FormData();
			data.append('upfile', file);
			xhr.open('POST', '/update');
			
			let percent = 0;
			xhr.upload.onloadstart = () => uprog.update(0);
			xhr.upload.onprogress = (evt) => uprog.update(Math.ceil((evt.loaded / evt.total) * 100));
			xhr.upload.onloadend = () => uprog.update(100);
			
			xhr.onload = function() {
				if (this.status == 507) {
					document.getElementById('message').innerHTML = "Not enough free storage for file!";
					reject(new Error('Not enough free storage'));
				} else if (this.status !== 201) {
					document.getElementById('message').innerHTML = this.response;
					reject(new Error(this.response));
				} else {
					uprog.update(100);
					document.getElementById('message').innerHTML = 'File uploaded!';
					updateFileList();
					getFreeStorage();
					resolve(this.response);
				}
			};
			
			xhr.onerror = () => reject(new Error('Network error'));
			xhr.send(data);
		});
	}
};
document.addEventListener("DOMContentLoaded", async () => {
	uprog.init();
	document.getElementById('up-label').onclick = uprog.upload;
	try {
		const response = await fetch('/version');
		if (response.status !== 200) {
			throw new Error('Failed to fetch version');
		}
		const data = await response.json();
		document.getElementById("fw").innerHTML += data.hub;
	} catch (e) {
		console.error(e);
	}
});
</script>
<style>
#message{font-size:1.3em;font-weight:bolder}
#up-file,.def-button{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:1.2em}
body{background:#3498db;font-family:sans-serif;font-size:1.3em;color:#777}
#up-file{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}
#up-bar,#up-progress{background-color:#f1f1f1;border-radius:10px;position:relative}
#up-bar{background-color:#3498db;width:0%;height:30px}
#up-wrap{background:#fff;max-width:36em;min-width:26em;margin:75px auto;padding:30px;border-radius:5px;text-align:center}
#up-label{background:#3498db;color:#fff;cursor:pointer}
#up-percent{position:absolute;top:6px;left:0;width:100%;display:flex;align-items:center;justify-content:center;text-shadow:-1px 1px 0 #000,1px 1px 0 #000,1px -1px 0 #000,-1px -1px 0 #000;color:#fff}
.home-button-container{display:flex;flex-direction:row;align-items: left;width: 20%;}
.def-button{background:#3498db;color:#fff;cursor:pointer;border:0;display:block;line-height:44px;text-decoration:none}</style>
</body>
</html>)";

// @brief Text of default index webpage
const char index_page[] = R"(<!DOCTYPE html>
<html lang='en-us'>
<head>
<title>Default Server Page</title>
</head>
<body>
<div id='up-wrap'>
<h1>Default Server Setup</h1>
<p>No index page found, this page will allow you to upload new web server files for your device. You will need to reboot to use a new index page.</p>
<p>Default files are: index.html, update.html, reset.html, the latter two are optional.</p>
<p><strong>Upload ALL necessary web files (.html/.css/.js) before rebooting</strong>.</p>
<div id='up-progress'>
	<div id='up-bar'></div>
	<div id='up-percent'>0%</div>
</div>
<div id='message'></div>
<input type='file' id='up-file' disabled>
<button class='def-button' id='up-button'>Upload</button>
<button class='def-button' id='restore'>Restore from Backup</button>
<a class='def-button' id='update' href='/update'>Update Firmware</a>
<button class='def-button' id='reboot'>Reboot Device</button>
<button class='def-button' id='reset'>Reset WiFi Settings</button>
</div>
<script>
let uprog = {
	hBar: null,
	hPercent: null,
	hFile: null,
	init: () => {
		uprog.hBar = document.getElementById('up-bar');
		uprog.hPercent = document.getElementById('up-percent');
		uprog.hFile = document.getElementById('up-file');
		uprog.hFile.disabled = false;
	},
	update: (percent) => {
		uprog.hBar.style.width = percent + '%';
		uprog.hPercent.innerHTML = percent + '%';
		if (percent === 100) uprog.hFile.disabled = false;
	},
	upload: (path) => {
		return new Promise((resolve, reject) => {
			if (uprog.hFile.files.length == 0) {
				reject(new Error('No file selected'));
				return;
			}
			
			let file = uprog.hFile.files[0];
			uprog.hFile.disabled = true;
			uprog.hFile.value = '';
			let xhr = new XMLHttpRequest(), data = new FormData();
			data.append('upfile', file);
			xhr.open('POST', '/upload-file');
			xhr.setRequestHeader('FILE_UPLOAD_PATH', '/www');
			
			let percent = 0;
			xhr.upload.onloadstart = () => uprog.update(0);
			xhr.upload.onprogress = (evt) => uprog.update(Math.ceil((evt.loaded / evt.total) * 100));
			xhr.upload.onloadend = () => uprog.update(100);
			
			xhr.onload = function() {
				if (this.status == 507) {
					document.getElementById('message').innerHTML = "Not enough free storage for file!";
					reject(new Error('Not enough free storage'));
				} else if (this.status !== 201) {
					document.getElementById('message').innerHTML = this.response;
					reject(new Error(this.response));
				} else {
					uprog.update(100);
					document.getElementById('message').innerHTML = 'File uploaded!';
					updateFileList();
					getFreeStorage();
					resolve(this.response);
				}
			};
			
			xhr.onerror = () => reject(new Error('Network error'));
			xhr.send(data);
		});
	}
};
window.addEventListener('load', uprog.init);

document.getElementById('up-button').onclick = uprog.upload;
	
document.getElementById("reboot").onclick = async function() {
	await PUTRequest("/reboot", "Success, rebooting!");	
};

document.getElementById("reset").onclick = async function() {
	await PUTRequest("/reset", "Reset successful!");
};

document.getElementById("restore").onclick = function() {
	restoreBackup();
};

async function restoreBackup() {
	if (document.getElementById("up-file").value === "") {
		return;
	}
	const selectedFile = document.getElementById("up-file").files[0];
	const reader = new FileReader();
	document.getElementById('message').innerHTML = 'Beginning restore...';
	reader.onload = async function(file) {
		let files;
		try {
			files = JSON.parse(file.target.result);
		} catch (e) {
			document.getElementById('message').innerHTML = e;
			return console.error(e);
		}
		
		for (let file in files) {
			try {
				await POSTRequest('/restorefile', 'File ' + file + ' restored', {"path": file, "contents": files[file]});
			} catch (e) {
				console.error(e);
			}
		}
		document.getElementById('message').innerHTML = 'Restore successful!';
	};
	reader.readAsText(selectedFile);
}

async function POSTRequest(path, successMessage, params = {}) {
	const data = new FormData();
	if (Object.keys(params).length !== 0) {
		for (let param in params) {
			data.append(param, params[param]);
		}
	}
	
	const response = await fetch(path, { method: 'POST', body: data });
	
	if (response.status !== 200) {
		const error = await response.json();
		document.getElementById('message').innerHTML = error;
		throw new Error(error);
	}
	
	document.getElementById('message').innerHTML = successMessage;
	return response.json();
}

// Send a PUT request with an optional object of key/value pairs for parameters
async function PUTRequest(path, successMessage, params = {}) {
	const data = new FormData();
	if (Object.keys(params).length !== 0) {
		for (let param in params) {
			data.append(param, params[param]);
		}
	}
	
	const response = await fetch(path, { method: 'PUT', body: data });
	
	if (response.status !== 200) {
		const error = await response.json();
		document.getElementById('message').innerHTML = error;
		throw new Error(error);
	}
	
	document.getElementById('message').innerHTML = successMessage;
	return response.json();
}
</script>
<style>
#message{font-size:1.3em;font-weight:bolder}
#up-file,.def-button{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:1.2em}
body{background:#3498db;font-family:sans-serif;font-size:1.3em;color:#777}
#up-file{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}
#up-bar,#up-progress{background-color:#f1f1f1;border-radius:10px;position:relative}
#up-bar{background-color:#3498db;width:0%;height:30px}
#up-wrap{background:#fff;max-width:36em;min-width:26em;margin:75px auto;padding:30px;border-radius:5px;text-align:center}
#up-percent{position:absolute;top:6px;left:0;width:100%;display:flex;align-items:center;justify-content:center;text-shadow:-1px 1px 0 #000,1px 1px 0 #000,1px -1px 0 #000,-1px -1px 0 #000;color:#fff}
.def-button{background:#3498db;color:#fff;cursor:pointer;border:0;display:block;line-height:44px;text-decoration:none}</style>
</body>
</html>)";