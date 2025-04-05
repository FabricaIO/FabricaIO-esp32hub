/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {

	// Attach button handlers
	document.getElementById("reboot").onclick = function() {
		PUTRequest('/reboot', 'Success, rebooting!');
	};

	document.getElementById("reset").onclick = function() {
		if (confirm("Reset WiFi settings?") == true) {
			PUTRequest('/reset', 'Success, rebooting!');
		}
	};
	
	document.getElementById("settime").onclick = function() {
		POSTRequest('/time', "Time set", { time:  Math.floor(new Date().getTime() / 1000) });
	};

});