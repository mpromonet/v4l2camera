<template>
  <div>
    <v-container>
      <v-row align="center" justify="space-around" >
          <v-btn v-if="!visibility" v-on:click="start">Start</v-btn>
          <v-btn v-if="visibility" v-on:click="stop">Stop</v-btn>
      </v-row>
    </v-container>
    <v-container>
      <v-row align="center" justify="center">
          <img v-if="visibility && image && !message" :src="image" />
          <video v-if="visibility && !image && !message" id="player" autoplay muted playsinline ></video>
          <div v-if="message">{{this.message}}</div>
      </v-row>
    </v-container>      
    <v-container>
      <v-row align="center" justify="center">
          {{ this.rtspinfo?.url }}
      </v-row>
    </v-container>
  </div>
</template>

<script>
import axios from "axios";
import JMuxer from "jmuxer";

export default {
  data() {
    return {
      image: "",
      visibility: true,
      ws: null,
      message: null,
      rtspinfo: null
    };
  },
  mounted() {
    axios.get("/api/rtspinfo").then(
      (response) => this.rtspinfo = response.data
    );
    axios.get("/api/isCapturing").then(
      (response) => this.visibility = response.data
    );
  },
  created() {
    console.log("Connecting WebSocket");
    const wsurl = document.location.href.replace("http", "ws") + "/ws";
    this.ws = new WebSocket(wsurl);
    this.ws.binaryType = 'arraybuffer';
    this.ws.onmessage = (message) => {
        const bytes = new Uint8Array(message.data);
        if ( (bytes.length > 1) && (bytes[0] === 255) && (bytes[1] === 216)) {
            // JPEG
            let binaryStr = "";
            for (let i = 0; i < bytes.length; i++) {
              binaryStr += String.fromCharCode(bytes[i]);
            }
            this.message = null;
            this.image = "data:image/jpeg;base64," + btoa(binaryStr);
            if (this.ws.jmuxer) {
              this.ws.jmuxer.destroy();
            }
            this.ws.jmuxer = null;
        } else if ( (bytes.length > 3) && (bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
            this.image = null;
            this.message = null;
            // H264
            if (!this.ws.jmuxer) {
              const videoElement = document.getElementById('player');
              videoElement.src = "";
              this.ws.jmuxer = new JMuxer({node: 'player', mode: 'video', readFpsFromTrack: true});
            }
            this.ws.jmuxer.feed({ video: bytes })
        } else {
            this.message = 'format not supported';
        }
    };
  },
  destroyed() {
    console.log("Closing WebSocket");
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  },
  methods: {
    start() {
      axios.get("/api/start").then( () => this.visibility = true);
    },
    stop() {
      axios.get("/api/stop").then( () => this.visibility = false);
    }
  }
};
</script>

<style>
</style>
