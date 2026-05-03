/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 * 
 * This file contains AJAX functions for POST, PUT, and GET methods, as well as a file upload handler.
 * Please ensure the correct HTML elements are present in your page to use these functions.
 */

// Send a POST request with an optional object of key/value pairs for parameters
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

// Send a GET request with an optional object of key value pairs for parameters
async function GETRequest(path, params = {}) {
	if (Object.keys(params).length !== 0) {
		let first = true;
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
	
	const response = await fetch(path);
	
	if (response.status !== 200) {
		const error = await response.json();
		document.getElementById('message').innerHTML = error;
		throw new Error(error);
	}
	
	document.getElementById('message').innerHTML = "";
	return response.json();
}

// File upload handler. Call uprog.init() first, then upload.upload()
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
			xhr.setRequestHeader('FILE_UPLOAD_PATH', path);
			
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