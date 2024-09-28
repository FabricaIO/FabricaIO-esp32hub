/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 * 
 * This file contains AJAX functions for POST, PUT, and GET methods, as well as a file upload handler.
 * Please ensure the correct HTML elements are present in your page to use these functions.
 */

// Send a POST request with an optional object of key/value pairs for parameters
// Returns the response to the optional callback provided
function POSTRequest(path, successMessage, params = {}, callback = null) {
	let xhr = new XMLHttpRequest(), data = new FormData();
	xhr.responseType = 'json';
	if (Object.keys(params).length !== 0 ) {
		for (let param in params) {
			data.append(param, params[param]);
		}
	}
	xhr.open('POST', path);
	xhr.onload = function() {
		if (this.status !== 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			document.getElementById('message').innerHTML = successMessage;
			if (callback !== null) {
				let response = xhr.response;
				console.log(response);
				callback(response);
			}
		}
	};
	xhr.send(data); 
}

// Send a PUT request with an optional object of key/value pairs for parameters
// Returns the response to the optional callback provided
function PUTRequest(path, successMessage, params = {}, callback = null) {
	let xhr = new XMLHttpRequest(), data = new FormData();
	xhr.responseType = 'json';
	if (Object.keys(params).length !== 0 ) {
		for (let param in params) {
			data.append(param, params[param]);
		}
	}
	xhr.open('PUT', path);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else {
			document.getElementById('message').innerHTML = successMessage;
			if (callback !== null) {
				let response = xhr.response;
				console.log(response);
				callback(response);
			}
		}
	};
	xhr.send(data);
}

// Send a GET request with an optional object of key value pairs for parameters
// Returns the response to the optional callback provided
function GETRequest(path, callback = null, params = {}) {
    let xhr = new XMLHttpRequest();
	xhr.responseType = 'json';
    if (Object.keys(params).length !== 0 ) {
		let first = true
		path += "?";
		for (let param in params) {
			if (first) {
				first = false;
			} else {
				path += "&";
			}
			path += param + "=" + params[param];
		}
	}
    xhr.open('GET', path);
	xhr.onload = function() {
		if (this.status != 200) {
			document.getElementById('message').innerHTML = this.response;
		} else if (callback !== null) {
			let response = xhr.response;
			console.log(response);
			callback(response);
		}
	};
	xhr.send();
}

// File upload handler. Call uprog.init() first then call uprog.upload("<path>")
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
		let Percent = percent + '%';
		uprog.hBar.style.width = Percent;
		uprog.hPercent.innerHTML = Percent;
		if (Percent == '100%') { uprog.hFile.disabled = false; }
	},
	upload: (path) => {
		if (uprog.hFile.files.length == 0) {
			return;
		}
		let file = uprog.hFile.files[0];
		uprog.hFile.disabled = true;
		uprog.hFile.value = '';
		let xhr = new XMLHttpRequest(), data = new FormData();
		data.append('upfile', file);
		xhr.open('POST', '/upload-file');
		xhr.setRequestHeader('FILE_UPLOAD_PATH', path);
		let percent = 0;
		xhr.upload.onloadstart = () => { uprog.update(0); };
		xhr.upload.onloadend = () => { uprog.update(100); };
		xhr.upload.onprogress = (evt) => {
			percent = Math.ceil((evt.loaded / evt.total) * 100);
			uprog.update(percent);
		};
		xhr.onload = function() {
			if (this.status == 507) {
				document.getElementById('message').innerHTML = "Not enough free storage for file!";
			} else if (this.status != 201) {
				document.getElementById('message').innerHTML = this.response;
			} else {
				uprog.update(100);
				document.getElementById('message').innerHTML = 'File uploaded!';
				updateFileList();
				getFreeStorage();
			}
		};
		xhr.send(data);
	}
};