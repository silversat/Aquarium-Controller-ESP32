
var ws = null;

function wsinit(subprotocols = '') {
	var url = 'ws://'+window.location.host+'/ws';
	
	if(ws && 'readyState' in ws && ws.readyState === 1) {
		ws.onclose = function(evt) { wsinit(subprotocols); };
		writeToScreen('Error: Overriding Existing Connection...');
		ws.close();
	} else {
		if(url.length > 0) {
			try {
				writeToScreen("url: '"+url+"'");
				writeToScreen("support: " + (window['MozWebSocket']?"MozWebSocket":"WebSocket"));

				if(subprotocols.length > 0) {
					var protocols = subprotocols.trim().replace(/\s/g, '');
					protocols =	(protocols.indexOf(',') > 0 ? protocols.split(',') : protocols);
					
					ws = window['MozWebSocket'] ? new MozWebSocket(url, protocols) : new WebSocket(url, protocols);
					writeToScreen("Socket Connection Open [req: '"+protocols+"']");
			} else {
					ws = window['MozWebSocket'] ? new MozWebSocket(url) : new WebSocket(url);
					writeToScreen("Socket Connection Open");
				}

				ws.onmessage = function(evt) {
					parseMessage(evt.data);
				};
				ws.onerror = function(error) {
					writeToScreen("Error: "+error.message);
				};
				ws.onclose = function(evt) {
//					writeToScreen("\nSocket Connection Closed");
//					wsinit(subprotocols);
				};
				ws.onopen = function() {
					onOpen(ws);
					writeToScreen("Socket Connection Opened with protocol: ['"+ws.protocol+"']");
				};
			} catch(exception) {
				writeToScreen("Exception: "+exception.message);
			}
		}
	}
}
