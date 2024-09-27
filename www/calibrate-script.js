/*
* This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

let sensorID = 0;
let calibrationStep = 0;

const calibrationResponse = Object.freeze({
	error: 0,
	done: 1,
	next: 2
});

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	const urlParams = new URLSearchParams(window.location.search);
	console.log(urlParams);
	const holder = document.getElementById('calstep');
	if (!urlParams.has('id')) {
		holder.innerHTML = '<h2>No device selected</h2>'
	} else {
		if (urlParams.has('step')) { 
			calibrationStep = parseInt(urlParams.get('step'));
		}
		sensorID = parseInt(urlParams.get('id'));
		doCalibrationStep(calibrationStep);
	}
});

// Runs a calibration step
function doCalibrationStep(step) {
	POSTRequest('/sensors/calibrate', 'Calibrating...', {"sensor": sensorID, "step": step}, receiveCalibrationResponse);
}

// Receives and processes the response of a calibration step
function receiveCalibrationResponse(response) {
	const holder = document.getElementById("calstep");
	if (response.response === calibrationResponse.error) {
		holder.innerHTML = '<h2>Calibration error!</h2><p>' + response.message + '</p>';
	} else if (response.response === calibrationResponse.done) {
		holder.innerHTML = '<h2>Calibration done!</h2><p>' + response.message + '</p><div class="button-container"><a href="/devices.html" class="def-button">Finish</a></div>';
	} else if (response.response === calibrationResponse.next) {
		let content = '<h2>Calibration Step ' + calibrationStep + ' Completed</h2>';
		calibrationStep++;
		content += '<p>' + response.message + '</p>'
		content += '<div class="button-container"><button class="def-button" onclick="doCalibrationStep(' + calibrationStep + ')">Next</button></div>';
		holder.innerHTML = content;
	}
}