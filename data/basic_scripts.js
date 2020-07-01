function $(id) {
	return document.getElementById(id);
}

function writeToScreen(message) {
	if($('log') && message.substr(0,4) != "TIME") {
		$('log').innerHTML += " - " + message;
	}
}

function mysubmit( formname ) {
	$(formname).submit();
}

function submitParam( formname, param, value ) {
	$(param).value = value;
	$(formname).submit();
}

function buttonAction( value ) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', '/runmodecontrol?control=' + value, true);		// true for async req
	xhr.send();
}
