<template>
  <div>
    <v-container>
      <v-row align="center" justify="space-around" >
          <v-btn v-if="!visibility" v-on:click="start">Start</v-btn>
          <v-btn v-if="visibility" v-on:click="stop">Stop</v-btn>
      </v-row>
    </v-container>
    <v-container>
      <v-row align="center" justify="center" style="height: 33vh;">
          <img v-if="visibility && image && !message" :src="image" class="h-100"/>
          <canvas v-show="visibility && !image && !message" id="player" class="h-100"></canvas>
          <div v-if="message" class="h-100">{{this.message}}</div>
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

export default {
  data() {
    return {
      image: "",
      visibility: true,
      ws: null,
      message: null,
      rtspinfo: null,
      videoCanvas: null,
      format: null
    };
  },
  mounted() {
    const video = document.getElementById("player");
    this.videoCanvas = video.getContext("2d");    
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
    this.ws.onmessage = async (message) => {
        if (typeof message.data === 'string') {
          this.format = JSON.parse(message.data);

        } else if (message.data instanceof ArrayBuffer) {
          const bytes = new Uint8Array(message.data);
          if ( (bytes.length > 1) && (bytes[0] === 255) && (bytes[1] === 216)) {
              // JPEG
              let binaryStr = "";
              for (let i = 0; i < bytes.length; i++) {
                binaryStr += String.fromCharCode(bytes[i]);
              }
              this.message = null;
              this.image = "data:image/jpeg;base64," + btoa(binaryStr);
              if (this.ws.decoder) {
                this.ws.decoder.close();
              }
              this.ws.decoder = null;
          } else if ( (bytes.length > 3) && (bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
              this.image = null;
              this.message = null;
              // H264
              if (!this.ws.decoder) {
                this.ws.decoder = new VideoDecoder({
                  output: (frame) => {
                    this.videoCanvas.canvas.width = frame.displayWidth;
                    this.videoCanvas.canvas.height = frame.displayHeight;
                    this.videoCanvas.drawImage(frame, 0, 0);
                    frame.close();
                  },
                  error: (e) => console.log(e.message),
                });
              }

              const naluType = bytes[4] & 0x1F;

              if (this.ws.decoder.state !== "configured" && naluType === 7) {
                  let codec = 'avc1.';
                  for (let i = 0; i < 3; i++) {
                      codec += ('00' + bytes[5+i].toString(16)).slice(-2);
                  }
                  const config = {codec};
                  const support = await VideoDecoder.isConfigSupported(config);
                  if (support.supported) {
                    this.ws.decoder.configure(config);
                  }
              } 
              if (this.ws.decoder.state === "configured") {
                  const chunk = new EncodedVideoChunk({
                      timestamp: new Date().getTime(),
                      type: (naluType === 7) ? "key" : "delta",
                      data: bytes,
                  });
                  this.ws.decoder.decode(chunk);
              }
          } else if (this.format) {
              try {
                const frame = new VideoFrame(bytes, {
                    format: this.format.format,
                    timestamp: performance.now(),
                    codedWidth: this.format.width,
                    codedHeight: this.format.height,
                });            
                this.videoCanvas.canvas.width = frame.displayWidth;
                this.videoCanvas.canvas.height = frame.displayHeight;
                this.videoCanvas.drawImage(frame, 0, 0);
                frame.close();            
              } catch (e) {
                this.message = 'format not supported';  
              }

          } else {
            this.message = 'format not supported';  
          }

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
