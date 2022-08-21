<template>
  <div>
    <div style="display: block">
      <button v-on:click="start">Start</button>
      <button v-on:click="stop">Stop</button>
    </div>
    <div style="display: block">
      <img v-if="visibility" :src="image" style="overflow: auto" />
      <video v-if="!image" id="player" controls autoplay muted></video>
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
    const ws = new WebSocket(wsurl);
    ws.binaryType = 'arraybuffer';
    ws.onmessage = (message) => {
        const bytes = new Uint8Array(message.data);
        if ((bytes[0] === 255) && (bytes[1] === 216)) {
            // JPEG
            let binaryStr = "";
            for (let i = 0; i < bytes.length; i++)
              binaryStr += String.fromCharCode(bytes[i]);
            this.image = "data:image/jpeg;base64," + btoa(binaryStr);
        } else if ((bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
            console.log(`size:${bytes.length} nal:${bytes[4]&0xf}`)
            // H264
            if (!ws.jmuxer) {
              ws.jmuxer = new JMuxer({node: 'player', flushingTime: 1000, debug: true});
            }
            ws.jmuxer.feed({ video: bytes })
        }
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
