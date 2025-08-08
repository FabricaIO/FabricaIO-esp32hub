/*
* This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	GETRequest("/config/", addSettings);
	GETRequest('/time', addTime);

});

// Adds all settings to page
function addSettings(config) {
	const holder = document.getElementById("settings");
	if (Object.keys(config).length === 0 ) {
		holder.innerHTML += '<p>No configurable options</p>';
	} else {
		for (opt in config) {
			let type = "text";
			let step = 1;
			let additionalAttrb = "";
			let name = opt.replace(" ", "_");
			if (typeof(config[opt]) === "object") {
				let newhtml = '<div class="stacked-input">\
				<label for="' + name + '">' + opt + '</label>\
				<select class="normal-input" name="' + name + '">';
				config[opt].options.forEach((value) => {
					if (value === config[opt].current) {
						newhtml += '<option class="normal-input" value="' + value + '" selected>' + value + '</option>';
					} else {
						newhtml += '<option class="normal-input" value="' + value + '">' + value + '</option>';
					}
				});
				newhtml += '</select></div>'
				holder.innerHTML += newhtml;
			} else {
				if (typeof(config[opt]) === "number") {
					type = "number";
					if (!Number.isInteger(config[opt])) {
						step = 0.01;
					}
				} else if (typeof(config[opt]) === "boolean") {
					type = "checkbox";
					if (config[opt] === true) {
						additionalAttrb= "checked";
					}
				} else if (typeof(config[opt]) === "string") {
					config[opt] = config[opt].replaceAll('"', '&quot;');
					if (name === "webPassword") {
						type = "password";
					}
				}
				holder.innerHTML += '<div class="stacked-input"><label for="' + name + '">' + opt + '</label>\
				<input class="normal-input" type="' + type + '" name="' + name + '" step="' + step + '" value="' + config[opt] + '" ' + additionalAttrb +'></div>';
			}
		}
	}
}

// Displays current time on device
function addTime(response) {
	const time = document.getElementById("time");
	var d = new Date(0);
	d.setUTCSeconds(response);
	time.innerHTML = d.toLocaleString();
}

// Parses and updates config for the sensor hub
function updateConfig() {
	inputs = document.querySelectorAll('#settings input');
	let new_config = {};
	Array.from(inputs).forEach((input) => {
		if (input.type === "number") {
			new_config[input.name] = input.valueAsNumber;
		} else if (input.type === "checkbox") {
			new_config[input.name] = input.checked;
		} else {
			new_config[input.name] = input.value;
		}
	});
	inputs = document.querySelectorAll('#device select');
	Array.from(inputs).forEach((input) => {
		new_config[input.name] = {"current": input.value};
	});
	console.log(new_config);
	POSTRequest("/config", "Settings updated!", {'save': true, 'config': JSON.stringify(new_config)});
}