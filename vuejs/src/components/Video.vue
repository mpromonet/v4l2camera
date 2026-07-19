<template>
  <v-card color="surface" elevation="4" rounded="lg">
    <!-- Toolbar row -->
    <v-card-item class="py-2 px-3">
      <template v-slot:prepend>
        <div class="d-flex align-center ga-2">
          <v-chip
            :color="wsConnected ? 'success' : 'warning'"
            variant="tonal"
            size="x-small"
            :prepend-icon="wsConnected ? 'mdi-wifi' : 'mdi-wifi-strength-1-alert'"
          >{{ wsConnected ? 'Live' : 'Connecting' }}</v-chip>
          <v-chip
            v-if="format.format"
            color="primary"
            variant="tonal"
            size="x-small"
            prepend-icon="mdi-film"
          >{{ format.format }}&nbsp;{{ format.width }}×{{ format.height }}</v-chip>
        </div>
      </template>
      <template v-slot:append>
        <v-btn
          :color="visibility ? 'error' : 'success'"
          :prepend-icon="visibility ? 'mdi-stop' : 'mdi-play'"
          variant="tonal"
          size="small"
          density="compact"
          @click="visibility ? stop() : start()"
        >{{ visibility ? 'Stop' : 'Start' }}</v-btn>
      </template>
    </v-card-item>

    <v-divider />

    <!-- Video area -->
    <div class="video-wrapper">
      <video
        v-show="visibility && !message"
        id="video"
        controls
        muted
        preload="none"
        class="video-el"
      ></video>
      <div
        v-if="!visibility || message"
        class="video-overlay d-flex flex-column align-center justify-center"
      >
        <v-icon
          size="72"
          :color="message ? 'error' : 'grey-darken-1'"
          class="mb-3"
        >{{ message ? 'mdi-alert-circle-outline' : 'mdi-camera-off' }}</v-icon>
        <span class="text-body-2 text-medium-emphasis mb-4">
          {{ message || 'Camera is stopped' }}
        </span>
        <v-btn
          v-if="!visibility"
          color="primary"
          variant="elevated"
          prepend-icon="mdi-play"
          @click="start"
        >Start Camera</v-btn>
      </div>
    </div>
  </v-card>
</template>

<script>
import axios from "axios";

export default {
  data() {
    return {
      visibility: true,
      ws: null,
      wsConnected: false,
      message: null,
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
    this.videoCanvas.clearRect(0, 0, videoCanvas.width, videoCanvas.height);
    axios.get("/api/isCapturing").then(
      (response) => this.visibility = response.data
    );
  },
  created() {
    let wsurl = new URL("./ws", import.meta.env.VITE_APP_BASE_URL || location.href);
    wsurl.protocol = wsurl.protocol.replace("http", "ws");
    this.connectWebSocket(wsurl.href);
  },
  destroyed() {
    if (this.ws && this.ws.decoder) {
      this.ws.decoder.close();
      this.ws.decoder = null;
    }
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  },
  methods: {
    start() {
      axios.get("/api/start").then(() => this.visibility = true);
    },
    stop() {
      axios.get("/api/stop").then(() => this.visibility = false);
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
      this.ws = new WebSocket(wsurl);
      this.ws.binaryType = 'arraybuffer';
      this.ws.onopen  = () => { this.wsConnected = true; };
      this.ws.onmessage = this.onMessage;
      this.ws.onclose = () => {
        this.wsConnected = false;
        setTimeout(() => this.connectWebSocket(wsurl), 1000);
      };
    },
    displayFrame(frame) {
      this.videoCanvas.canvas.width  = frame.displayWidth;
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
          codec += ('00' + bytes[5 + i].toString(16)).slice(-2);
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
          type: "key",
          data: bytes,
        });
        this.ws.decoder.decode(chunk);
        return await new Promise(r => this.frameResolved = r);
      } else {
        return Promise.reject(`H264 decoder not configured`);
      }
    },
    async onJPEGFrame(data) {
      const decoder = new ImageDecoder({data, type: 'image/jpeg'});
      const image = await decoder.decode();
      return new VideoFrame(image.image, {timestamp: performance.now()});
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
      if ((bytes.length > 1) && (bytes[0] === 255) && (bytes[1] === 216)) {
        return this.onJPEGFrame(bytes);
      } else if ((bytes.length > 3) && (bytes[0] === 0) && (bytes[1] === 0) && (bytes[2] === 0) && (bytes[3] === 1)) {
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

<style scoped>
.video-wrapper {
  position: relative;
  width: 100%;
  aspect-ratio: 16 / 9;
  background: #050c12;
  overflow: hidden;
}
.video-el {
  width: 100%;
  height: 100%;
  object-fit: contain;
  display: block;
}
.video-overlay {
  position: absolute;
  inset: 0;
}
</style>
