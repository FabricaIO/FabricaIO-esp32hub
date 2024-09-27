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
});

// Update the list of files displayed on this page
function updateFileList() {
	document.getElementById("file-list").innerHTML = "";
	getFileList("/", 5);
}

// Get list of files
function getFileList(filePath, traverseDepth = 0) {
	GETRequest("/list", addFileList, { path: filePath, depth: traverseDepth });
}

// Callback for receiving file list data
function addFileList(response) {
	if (response != null) {
		let list = document.getElementById("file-list");
		for (let i = 0; i < response.files.length; i++)
		{
			list.innerHTML += `
			<tr class="file">
				<td>` + response.files[i] + `</td>
				<td class="download"><a href="/download?path=` + response.files[i] + `">Download</a>
				<td class="delete" onclick="deleteFile(this)" data-name="` + response.files[i] + `">Delete</td>
			</tr>`;
		}
	}
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