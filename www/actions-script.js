/*
* This file is licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* Contributors: Sam Groveman
*/

// Run code when page DOM is loaded
document.addEventListener("DOMContentLoaded", () => {
	GETRequest("/actors/", addActors);
});


// Adds all actors with their actions to the page
function addActors(actors) {
	console.log(actors);
	const holder = document.getElementById("actions");
	let html = '';
	if (actors.actors.length === 0) {
		html ='<p>No available actions</p>';
	} else {
	    html = '<table><tr><th>Actor</th><th>Action</th><th>Execute</th></tr>';
		actors.actors.forEach(actor => {
			actor.actions.forEach(action =>{
				html += '<tr><td>' + actor.description.name + '</td><td>' + action + '</td><td><a class="def-button" onClick="executeAction(\'' + actor.description.name + '\',\'' + action + '\')">Execute</a></td></tr>';
			});
		});
		html += '</table>';
	}
	holder.innerHTML = html;
}

// Adds an action to the execution queue with any payload
function executeAction(actor, action) {
	document.getElementById("message").innerHTML ="";
	POSTRequest("/actors/add", "Action added to queue", {actorName: actor, name: action, payload: document.getElementById("payload").value});
}