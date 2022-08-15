<template>
  <div>
    <div style="display: block">
      <button v-on:click="start">Start</button>
      <button v-on:click="stop">Stop</button>
    </div>
    <div style="display: block">
      <img v-if="visibility" :src="image" style="overflow: auto" />
      <video id="player"></video>
    </div>
  </div>
</template>

<script>
import axios from "axios";
import JMuxer from 'jmuxer';

var serviceurl = "";

export default {
  data: function () {
    return {
      image: "",
      visibility: true
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
          const bytes = new Uint8Array(binary);
          if ((bytes[0] === 255) && (bytes[1] === 216)) {
            // JPEG
            let binaryStr = "";
            for (let i = 0; i < bytes.length; i++)
              binaryStr += String.fromCharCode(bytes[i]);
            this.image = "data:image/jpeg;base64," + btoa(binaryStr);
          } else if ((bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
            console.log(`nal:${bytes[4]&0xf}`)
            // H264
            if (!connection.jmuxer) {
              connection.jmuxer = new JMuxer({node: 'player', debug: true});
            }
            connection.jmuxer.feed({ video: bytes, duration: 10 })
          }
        });
      }
    };

    connection.onopen = function (event) {
      console.log("Successfully connected to websocket ...");
    };
  },
  methods: {
    start: function() {
      axios.get(serviceurl + "/api/start");
      this.visibility = true;
    },
    stop: function() {
      axios.get(serviceurl + "/api/stop");
      this.visibility = false;
    }
  }
};
</script>