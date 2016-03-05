var out = function(message) {
	var div = document.createElement('div');
	div.innerHTML = message;
	document.getElementById('output').appendChild(div);
};

window.onload = function() {
	var url = 'ws://' + location.host + '/ws';

	websocket = new WebSocket(url);
	websocket.binaryType = 'arraybuffer';
	websocket.onopen = function(ev) {
		out('CONNECTED');
	};
	websocket.onclose = function(ev) {
		out('DISCONNECTED');
	};
	websocket.onmessage = function(ev) {
		if (!ev.data) {
			out('<span style="color: blue;">PING... </span>');
		} else if (typeof(ev.data)=="string") {
			out('<span style="color: blue;">RECV:'+ev.data+'</span>');
		} else {	      
			var bytes = new Uint8Array(ev.data);	
			var binary = "";
			for(i=0;i<bytes.length;i++) binary += String.fromCharCode(bytes[i]);

			var img = document.getElementById('wsimage');
			img.src = 'data:image/jpeg;base64,' + btoa(binary);
		}
	};
	websocket.onerror = function(ev) {
		out('<span style="color: red; ">ERROR: ' + ev.data + '</span> ');
	};
};
