<template>
  <div>
    <div>
      <v-btn v-on:click="start">Start</v-btn>
      <v-btn v-on:click="stop">Stop</v-btn>
    </div>
    <div>
      <img class="video" v-if="visibility" :src="image" style="overflow: auto" />
      <video class="video" v-if="visibility && !image" id="player" autoplay muted playsinline ></video>
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
      visibility: true,
      ws: null,
    };
  },
  created: function () {
    console.log("Connecting WebSocket");
    const wsurl = document.location.href.replace("http", "ws") + "/ws";
    this.ws = new WebSocket(wsurl);
    this.ws.binaryType = 'arraybuffer';
    this.ws.onmessage = (message) => {
        const bytes = new Uint8Array(message.data);
        if ((bytes[0] === 255) && (bytes[1] === 216)) {
            // JPEG
            let binaryStr = "";
            for (let i = 0; i < bytes.length; i++) {
              binaryStr += String.fromCharCode(bytes[i]);
            }
            this.image = "data:image/jpeg;base64," + btoa(binaryStr);
            this.ws.jmuxer = null;
        } else if ((bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
            this.image = "";
            // H264
            if (!this.ws.jmuxer) {
              this.ws.jmuxer = new JMuxer({node: 'player', mode: 'video', readFpsFromTrack: true, flushingTime: 1000});
            }
            this.ws.jmuxer.feed({ video: bytes })
        }
    };
  },
  destroyed: function() {
    console.log("Closing WebSocket");
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
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

<style>
.video {
  margin: auto;
  display: flex;
}
</style>