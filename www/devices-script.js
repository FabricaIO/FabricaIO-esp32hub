/*
* This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	GETRequest("/sensors/", addSensors);
	GETRequest("/signals/", addReceivers);

});

// Adds all connected sensors to the page
function addSensors(sensors) {
	const holder = document.getElementById("sensors");
	if (sensors.sensors.length === 0) {
		holder.innerHTML ='<p>No active sensors</p>';
	} else {
		holder.innerHTML = '<div class="button-container">\r\n';
		sensors.sensors.forEach(sensor => {
			holder.innerHTML += '<button class="def-button" onclick="loadDevice(true,\'' + sensor.description.name + '\',' +  sensor.positionID +')">' + sensor.description.name + '</button>';
		});
	}
}

// Adds all connected signal receivers to the page
function addReceivers(receivers) {
	const holder = document.getElementById("receivers");
	if (receivers.receivers.length === 0) {
		holder.innerHTML = '<p>No active receivers</p>';
	} else {
		holder.innerHTML = '<div class="button-container">\r\n';
		receivers.receivers.forEach(receiver => {
			holder.innerHTML += '<button class="def-button" onclick="loadDevice(false,\'' + receiver.description.name + '\',' +  receiver.positionID +')">' + receiver.description.name + '</button>';
		});
	}
}

// Adds a signal receiver to the page for configuring
function loadDevice(isSensor, name, posID) {
	const holder = document.getElementById("device");
	holder.dataset.posid = posID;
	holder.dataset.sensor = isSensor;
	device.data
	holder.innerHTML = '<h2>' + name + '</h2>';
	let path = "/signals/config";
	let dataName = "receiver";
	if (isSensor) {
		path = "/sensors/config";
		dataName = "sensor";
	}
	data = {};
	data[dataName] = posID;
	GETRequest(path, addDeviceConfig, data);
}

// Adds a device configuration to the page
function addDeviceConfig(device) {
	const holder = document.getElementById("device");
	if (Object.keys(device).length === 0 ) {
		holder.innerHTML += '<p>No configurable options</p>';
	} else {
		for (opt in device) {
			let type = "text";
			let step = 1;
			let additionalAttrb = "";
			let name = opt.replace(" ", "_");
			if (typeof(device[opt]) === "object") {
				let newhtml = '<div class="stacked-input">\
				<label for="' + name + '">' + opt + '</label>\
				<select class="normal-input" name="' + name + '">';
				device[opt].options.forEach((value) => {
					if (value === device[opt].current) {
						newhtml += '<option class="normal-input" value="' + value + '" selected>' + value + '</option>';
					} else {
						newhtml += '<option class="normal-input" value="' + value + '">' + value + '</option>';
					}
				});
				newhtml += '</select></div>'
				holder.innerHTML += newhtml;
			} else {
				if (typeof(device[opt]) === "number") {
					type = "number";
					if (!Number.isInteger(device[opt])) {
						step = 0.01;
					}
				} else if (typeof(device[opt]) === "boolean") {
					type = "checkbox";
					if (device[opt] === true) {
						additionalAttrb= "checked";
					}
				} else if (typeof(device[opt]) === "string") {
					device[opt] = device[opt].replaceAll('"', '&quot;');
				}
				holder.innerHTML += '<div class="stacked-input"><label for="' + name + '">' + opt + '</label>\
				<input class="normal-input" type="' + type + '" name="' + name + '" step="' + step + '" value="' + device[opt] + '" ' + additionalAttrb +'></div>';
			}
		}
		holder.innerHTML += '<div class="button-container"><button class="def-button" onclick="updateDeviceConfig(' + holder.dataset.sensor + ',' + holder.dataset.posid + ')">Update Config</button></div>';
	}
}

// Parses and updates config for device
function updateDeviceConfig(isSensor, posID) {
	let inputs = document.querySelectorAll('#device input');
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
	if (isSensor) {
		POSTRequest('/sensors/config', "Device config updated!", {'sensor': posID, 'config': JSON.stringify(new_config)});
	} else {
		POSTRequest('/signals/config', "Device config updated!", {'receiver': posID, 'config': JSON.stringify(new_config)});
	}
}