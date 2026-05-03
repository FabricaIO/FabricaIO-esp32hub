/*
* This file is licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* Contributors: Sam Groveman
*/

var autoMeasure;

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	document.getElementById("refresh").onclick = function() {
		updateParameters();
	}
	document.getElementById("automeas").onclick = function() {
		if (document.getElementById('automeas').checked) {
			autoMeasure = setInterval(function () {
				updateParameters();
			}, 500); 
		} else {
			clearInterval(autoMeasure);
		}
	}
	GETRequest("sensors/measurement", addParameters);

});

// Gets parameters from the device
async function updateParameters() {
	let response;
	try {
		response = await GETRequest("/sensors/measurement?update");
	} catch (e) {
		document.getElementById("message").html = "Could not fetch data";
		return console.error(e);
	}
	document.getElementById("message").html = "";
	addParameters(response);
}

// Adds all connected sensors to the page
function addParameters(parameters) {
	const holder = document.getElementById("parameters");
	let html = '';
	if (parameters.measurements.length === 0) {
		html ='<p>No active sensors</p>';
	} else {
	    html = '<table><tr><th>Name</th><th>Parameter</th><th>Value</th><th>Unit</th></tr>';
		parameters.measurements.forEach(parameter => {
				html += '<tr><td>' + parameter.name + '</td><td>' + parameter.parameter + '</td><td>' + parameter.value + '</td><td>' + parameter.unit + '</td></tr>';
		});
		html += '</table>';
	}
	holder.innerHTML = html;
}
