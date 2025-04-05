/*
* This file is licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* Contributors: Sam Groveman
*/

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	document.getElementById("refresh").onclick = function() {
		GETRequest("/sensors/measurement?update", addParameters);
	}
	GETRequest("sensors/measurement", addParameters);

});


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
