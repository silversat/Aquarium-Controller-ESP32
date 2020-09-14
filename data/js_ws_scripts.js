
function parseMessage( msg ) {
	var sep = msg.indexOf('=');
	var key = msg.substr(0,sep).toLowerCase();
	var value = msg.substr(sep+1);
	if($(key)) {
		$(key).innerHTML = value;
	}
}

function onOpen(ws) {
}
