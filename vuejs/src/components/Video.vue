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
          <video v-show="visibility && !message" id="video" muted class="h-100" preload="none"></video>
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

export default {
  data() {
    return {
      visibility: true,
      ws: null,
      message: null,
      rtspinfo: null,
      videoCanvas: null,
      format: {format: "", width: 0, height: 0},
      frameResolved: null
    };
  },
  mounted() {
    const videoCanvas = document.createElement("canvas");
    this.videoCanvas = videoCanvas.getContext("2d");
    const video = document.getElementById("video");
    video.srcObject = videoCanvas.captureStream();
    video.play();
    this.videoCanvas.clearRect(0,0,videoCanvas.width,videoCanvas.height);
    axios.get("/api/rtspinfo").then(
      (response) => this.rtspinfo = response.data
    );
    axios.get("/api/isCapturing").then(
      (response) => this.visibility = response.data
    );
  },
  created() {
    let wsurl = new URL("./ws", location.href);
    wsurl.protocol = wsurl.protocol.replace("http","ws");
    this.connectWebSocket(wsurl.href);
  },
  destroyed() {
    console.log("Closing decoder");
    if (this.ws.decoder) {
      this.ws.decoder.close();
    }
    this.ws.decoder = null;
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
    },
    async onMessage(message) {
      const { data } = message;
      if (typeof data === 'string') {
        this.format = JSON.parse(data);

      } else if (data instanceof ArrayBuffer) {
        const bytes = new Uint8Array(data);
        try {
          const frame = await this.onFrame(bytes);
          this.displayFrame(frame);
          this.message = null;
        } catch (e) {
          this.message = e;  
        }
      }      
    },
    connectWebSocket(wsurl) {
        console.log(`Connecting WebSocket to ${wsurl}`);
        this.ws = new WebSocket(wsurl);
        this.ws.binaryType = 'arraybuffer';
        this.ws.onmessage = this.onMessage;
        this.ws.onclose = () => setTimeout(() => this.connectWebSocket(wsurl), 1000);
    },
    displayFrame(frame) {
        this.videoCanvas.canvas.width = frame.displayWidth;
        this.videoCanvas.canvas.height = frame.displayHeight;
        this.videoCanvas.drawImage(frame, 0, 0);
        frame.close();       
    },
    async onH264Frame(bytes) {
      if (!this.ws.decoder) {
        this.ws.decoder = new VideoDecoder({
          output: (frame) => this.frameResolved(frame),
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
          } else {
            return Promise.reject(`${codec} is not supported`);
          }
      } 
      if (this.ws.decoder.state === "configured") {
          const chunk = new EncodedVideoChunk({
              timestamp: performance.now(),
              type: (naluType === 7) || (naluType === 5) ? "key" : "delta",
              data: bytes,
          });
          this.ws.decoder.decode(chunk);
          return await new Promise(r => this.frameResolved = r);
      } else {
        return Promise.reject(`H264 decoder not configured`);
      }
    },
    async onJPEGFrame(bytes) {
      let binaryStr = "";
      for (let i = 0; i < bytes.length; i++) {
        binaryStr += String.fromCharCode(bytes[i]);
      }
      const img = new Image();
      img.src = "data:image/jpeg;base64," + btoa(binaryStr);
      await new Promise(r => img.onload=r);
      return new VideoFrame(img, {timestamp: performance.now()});
    },
    onDefaultFrame(bytes) {
      return new VideoFrame(bytes, {
                  format: this.format.format,
                  timestamp: performance.now(),
                  codedWidth: this.format.width,
                  codedHeight: this.format.height,
        });    
    },
    onFrame(bytes) {
      if ( (bytes.length > 1) && (bytes[0] === 255) && (bytes[1] === 216)) {
        return this.onJPEGFrame(bytes);

      } else if ( (bytes.length > 3) && (bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
        return this.onH264Frame(bytes);

      } else if (this.format) {
        return this.onDefaultFrame(bytes);

      } else {
        return Promise.reject(`Unknown format`);  
      }
    }
  }
};
</script>

<style>
</style>
