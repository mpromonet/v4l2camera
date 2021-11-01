<template>
  <div>
    <div style="display: block">
      <button v-on:click="start">Start</button>
      <button v-on:click="stop">Stop</button>
    </div>
    <div style="display: block">
      <img :src="image" style="overflow: auto" />
    </div>
  </div>
</template>

<script>
import axios from "axios";

var serviceurl = "";

export default {
  data: function () {
    return {
      image: "",
    };
  },
  created: function () {
    console.log("Starting connection to WebSocket Server");
    const wsurl = document.location.href.replace("http", "ws") + "/ws";
    const connection = new WebSocket(wsurl);

    connection.onmessage = (message) => {
      let data = message.data;
      if (data instanceof Blob) {
        new Response(data).arrayBuffer().then((binary) => {
          var bytes = new Uint8Array(binary);
          var binaryStr = "";
          for (let i = 0; i < bytes.length; i++)
            binaryStr += String.fromCharCode(bytes[i]);
          this.image = "data:image/jpeg;base64," + btoa(binaryStr);
        });
      }
    };

    connection.onopen = function (event) {
      console.log("Successfully connected to websocket ...");
    };
  },
  methods: {
    start: function() {
      axios.get(serviceurl + "/api/start")
    },
    stop: function() {
      axios.get(serviceurl + "/api/stop")
    }
  }
};
</script>