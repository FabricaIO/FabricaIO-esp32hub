/*
* This file is licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	// Initialize the upload handler
	uprog.init();

	// Get server info
	getFreeStorage();
	updateFileList();

	// Attach button handlers
	document.getElementById("up-www").onclick = function() {
		uprog.upload('/www');
	};
	document.getElementById("backup").onclick = function() {
		downloadBackup();
	};
	document.getElementById("restore").onclick = function() {
		restoreBackup();
	};
});

// Update the list of files displayed on this page
function updateFileList() {
	document.getElementById("file-list").innerHTML = "";
	getFileList("/", 5);
}

// Get list of files
function getFileList(filePath, traverseDepth = 0) {
	GETRequest("/list", addFileList, { path: filePath, depth: 0 });
	GETRequest("/list", addDirs, { path: filePath, type: 1, depth: traverseDepth });
}

let filesLoaded = false;

// Processes each directory one by one
async function addDirs(response) {
	if (response != null) {
		for (let i = 0; i < response.list.length; i++)
		{
			while (!filesLoaded) {
				await new Promise(r => setTimeout(r, 10));
			}
			filesLoaded = false;
			GETRequest("/list", addFileList, { path: response.list[i], type: 0, depth: 0 });
		}
	}
}

// Callback for receiving file list data
async function addFileList(response) {
	if (response != null) {
		let list = document.getElementById("file-list");
		for (let i = 0; i < response.list.length; i++)
		{
			list.innerHTML += `
			<tr class="file">
				<td>` + response.list[i] + `</td>
				<td class="download"><a href="/download?path=` + response.list[i] + `">Download</a>
				<td class="delete" onclick="deleteFile(this)" data-name="` + response.list[i] + `">Delete</td>
			</tr>`;
		}
	}
	await new Promise(r => setTimeout(r, 10));
	filesLoaded = true;
}

// Delete file
function deleteFile(file) {
	let name = file.dataset.name;
	if (confirm("Delete " + name + "?")) {
		POSTRequest("/delete", "File deleted!", { path: name }, fileDeleted);
	}
}

// Callback for file being deleted
function fileDeleted(response) {
	let file = document.querySelector('[data-name="' + response.file + '"]');
	document.getElementById('message').innerHTML = 'File deleted!';
	file.parentNode.remove();
	getFreeStorage()
}

// Gets free storage space on device
function getFreeStorage() {
	GETRequest("/freeSpace", addFreeSpace);
}

// Callback for receiving free storage space
function addFreeSpace(response) {
	if (response != null) {
		let space = document.getElementById("freespace");
		space.innerHTML = 'Free space: ' + response.space + ' bytes';
	}
}

// Prepares a JSON file of all the config files for download
async function downloadBackup() {
	const files = document.querySelectorAll('.file');
	let backups = {};
	let dots = 1;
	for (const file of files) {
		document.getElementById('message').innerHTML = 'Backing up, please wait'
		for (let i = 0; i < dots; i++) {
			document.getElementById('message').innerHTML += '.';
		}
		if (dots == 4) {
			dots = 0;
		} else {
			dots++;
		}
		let response = await fetch(file.querySelector('.download a').getAttribute('href'));
		if (!response.ok) {
			console.log(`Response status: ${response.status}`);
			document.getElementById('message').innerHTML = 'Could not complete backup';
			return;
		}
		backups[file.querySelector('.delete').dataset.name] = await response.text();
		await new Promise(r => setTimeout(r, 50));
	}
	const a = document.createElement('a');
	a.href = URL.createObjectURL( new Blob([JSON.stringify(backups, null, 2)], { type:'application/json' }) );
	a.download = "Backup.json";
	a.click();
	a.remove();
	document.getElementById('message').innerHTML = 'Backup successful!'
}

// Restores a backup from a JSON file
async function restoreBackup() {
	const selectedFile = document.getElementById("up-file").files[0];
	const reader = new FileReader();
	document.getElementById('message').innerHTML = 'Beginning restore...';
	reader.onload = async function(file) {
		let files = JSON.parse(file.target.result);
		let restored = false;
		for (let file in files) {
			POSTRequest('/restorefile', 'File ' + file + ' restored', {"path": file, "contents": files[file]}, async function() {
				await new Promise(r => setTimeout(r, 50))
				restored = true;
			});
			// Wait for file to be restored before proceeding
			while (!restored) {
				await new Promise(r => setTimeout(r, 50));
			}
			restored = false;
		}
		document.getElementById('message').innerHTML = 'Restore successful!';
		getFreeStorage();
		updateFileList();
	};
	reader.readAsText(selectedFile);
}