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

async function getFileList(filePath, traverseDepth = 0) {
	try {
		const [fileListResponse, dirsResponse] = await Promise.all([
			GETRequest("/list", { path: filePath, depth: 0 }),
			GETRequest("/list", { path: filePath, type: 1, depth: traverseDepth })
		]);
		
		addFileList(fileListResponse);
		addDirs(dirsResponse);
	} catch (e) {
		console.error(e);
	}
}

let filesLoaded = false;

// Processes each directory one by one
async function addDirs(response) {
	if (response != null) {
		const requests = response.list.map(dirPath => 
			GETRequest("/list", { path: dirPath, type: 0, depth: 0 })
		);
		const results = await Promise.all(requests);
		results.forEach(fileListResponse => addFileList(fileListResponse));
	}
}

// Callback for receiving file list data
function addFileList(response) {
	if (response == null || response.list.length === 0) {
		return;
	}
	
	let list = document.getElementById("file-list");
	const fragment = document.createDocumentFragment();
	
	for (let i = 0; i < response.list.length; i++) {
		const fileName = response.list[i];
		const tr = document.createElement("tr");
		tr.className = "file";
		tr.innerHTML = `
			<td>${fileName}</td>
			<td class="download"><a href="/download?path=${fileName}">Download</a></td>
			<td class="delete" data-name="${fileName}">Delete</td>
		`;
		tr.querySelector(".delete").addEventListener("click", function() {
			deleteFile(this);
		});
		fragment.appendChild(tr);
	}
	
	list.appendChild(fragment);
}

// Delete file
async function deleteFile(file) {
	let name = file.dataset.name;
	if (confirm("Delete " + name + "?")) {
		try {
			const response = await POSTRequest("/delete", "File deleted!", { path: name });
			
			// Remove the file from the DOM
			const fileElement = document.querySelector('[data-name="' + response.file + '"]');
			fileElement.parentNode.remove();
			
			document.getElementById('message').innerHTML = 'File deleted!';
			getFreeStorage();
		} catch (err) {
			console.error("Delete failed:", err);
		}
	}
}

// Gets free storage space on device
async function getFreeStorage() {
	try {
		const response = await GETRequest("/freeSpace");
		let space = document.getElementById("freespace");
		space.innerHTML = 'Free space: ' + response.space + ' bytes';	
	} catch (err) {
		console.error("Failed to fetch free storage:", err);
	}
}

// Prepares a JSON file of all the config files for download
async function downloadBackup() {
	const files = document.querySelectorAll('.file');
	let backups = {};
	let dots = 0;
	for (const file of files) {
		// Update progress with animated dots
		dots = (dots % 3) + 1;
		document.getElementById('message').innerHTML = 'Backing up, please wait' + '.'.repeat(dots);
		
		// Download file
		const downloadLink = file.querySelector('.download a').getAttribute('href');
		const response = await fetch(downloadLink);
		
		if (!response.ok) {
			console.error(`Download failed: ${response.status}`);
			document.getElementById('message').innerHTML = 'Could not complete backup';
			return;
		}
		
		const fileName = file.querySelector('.delete').dataset.name;
		backups[fileName] = await response.text();
		// Avoid spamming MCU
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
	if (document.getElementById("up-file").value === "") {
		return;
	}
	const selectedFile = document.getElementById("up-file").files[0];
	const reader = new FileReader();
	document.getElementById('message').innerHTML = 'Beginning restore...';
	reader.onload = async function(file) {
		let files;
		try {
			files = JSON.parse(file.target.result);
		} catch (e) {
			document.getElementById('message').innerHTML = e;
			return console.error(e);
		}
        console.log(files);
		for (let filePath in files) {
			await POSTRequest('/restorefile', 'File ' + filePath + ' restored', {"path": filePath, "contents": files[filePath]});
			// Avoid spamming MCU too quickly
			await new Promise(r => setTimeout(r, 50));
		}
		document.getElementById('message').innerHTML = 'Restore successful!';
		getFreeStorage();
		updateFileList();
	};
	reader.readAsText(selectedFile);
}