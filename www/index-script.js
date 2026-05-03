/*
 * This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * Contributors: Sam Groveman
 */

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {

	// Attach button handlers
	document.getElementById("reboot").onclick = async function() {
		try {
			await PUTRequest('/reboot', 'Success, rebooting!');
		} catch (e) {
			console.error(e);
		}
	};

	document.getElementById("reset").onclick = async function() {
		if (confirm("Reset WiFi settings?") == true) {
			try {
				PUTRequest('/reset', 'Success, rebooting!');
			} catch (e) {
				console.error(e);
			}
		}
	};
	
	document.getElementById("settime").onclick = async function() {
		try {
			POSTRequest('/time', "Time set", { time:  Math.floor(new Date().getTime() / 1000) });
		} catch (e) {
			console.error(e);
		}
	};

});