<template>
<div>
    <image :src="image">
</div>    
</template>

<script>
export default {
  data: function() {
    return {
      connection: null,
      image: ""
    }
  },
  created: function() {
    console.log("Starting connection to WebSocket Server")
    const wsurl = document.location.href.replace("http","ws") + "/ws"; 
    this.connection = new WebSocket(wsurl)

    this.connection.onmessage = function(message) {
      	let data = message.data;
		if (data instanceof Blob) {
			new Response(data).arrayBuffer().then( (binary) => {
				var bytes = new Uint8Array(binary);	
				var binaryStr = "";
				for(let i=0;i<bytes.length;i++) binaryStr += String.fromCharCode(bytes[i]);
				this.image = 'data:image/jpeg;base64,' + btoa(binaryStr);
			})
		}
    }

    this.connection.onopen = function(event) {
      console.log("Successfully connected to websocket ...")
    }

  }
}
</script>