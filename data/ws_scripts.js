
var s_url = 'ws://'+window.location.host+"/ws";
var s_protocol = null;
var ws;

function wsinit(protocol) {
	if(protocol) s_protocol = protocol;
	try {
		ws = new WebSocket(s_url, s_protocol);
		ws.onmessage = function(evt) {
//			writeToScreen(evt.data);
			parseMessage(evt.data);
		};
		ws.onerror = function(error) {
			writeToScreen("Error: "+error.message);
		};
		ws.onclose = function(evt) {
			writeToScreen("\nSocket Connection Closed");
//			wsinit(s_protocol);
		};
		ws.onopen = function() {
			writeToScreen("Socket Connection Opened [req: "+s_protocol+"], ack: ["+ws.protocol+"]\n");
		};
	} catch(exception) {
		writeToScreen("Exception: "+exception.message);
	}
}

function parseMessage( msg ) {
	var sep = msg.indexOf('=');
	var key = msg.substr(0,sep).toLowerCase();
	var value = msg.substr(sep+1);
	if($(key)) {
		$(key).innerHTML = value;
	}
}
