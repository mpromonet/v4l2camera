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

// byte offsets of each 4:2:2 packed sample's y0/u/y1/v components within its 4-byte group
const PACKED_422_LAYOUTS = {
  YUYV: { y0: 0, u: 1, y1: 2, v: 3 },
  YUY2: { y0: 0, u: 1, y1: 2, v: 3 },
  YVYU: { y0: 0, v: 1, y1: 2, u: 3 },
  UYVY: { u: 0, y0: 1, v: 2, y1: 3 },
  VYUY: { v: 0, y0: 1, u: 2, y1: 3 },
};

export default {
  data() {
    return {
      visibility: true,
      ws: null,
      wsConnected: false,
      message: null,
      videoCanvas: null,
      format: {format: "", width: 0, height: 0},
      frameResolvers: []
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
          output: (frame) => this.frameResolvers.shift()?.(frame),
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
        return await new Promise(r => this.frameResolvers.push(r));
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
      const packed422 = PACKED_422_LAYOUTS[this.format.format];
      if (packed422) {
        return this.packed422ToVideoFrame(bytes, packed422);
      }
      if (this.format.format === 'NV21') {
        return this.nv21ToVideoFrame(bytes);
      }
      if (this.format.format === 'RGB3') {
        return this.rgb24ToVideoFrame(bytes, 'RGBA');
      }
      if (this.format.format === 'BGR3') {
        return this.rgb24ToVideoFrame(bytes, 'BGRA');
      }
      return new VideoFrame(bytes, {
        format: this.format.format,
        timestamp: performance.now(),
        codedWidth: this.format.width,
        codedHeight: this.format.height,
      });
    },
    nv21ToVideoFrame(bytes) {
      // NV21 (Y plane + interleaved V/U plane) isn't a valid VideoFrame format, convert to I420
      const { width, height } = this.format;
      const ySize = width * height;
      const chromaSize = (width / 2) * (height / 2);
      const i420 = new Uint8Array(ySize + chromaSize * 2);
      i420.set(bytes.subarray(0, ySize), 0);
      const uOffset = ySize;
      const vOffset = ySize + chromaSize;
      for (let i = 0; i < chromaSize; i++) {
        i420[vOffset + i] = bytes[ySize + i * 2];     // NV21 order: V then U
        i420[uOffset + i] = bytes[ySize + i * 2 + 1];
      }
      return new VideoFrame(i420, {
        format: 'I420',
        timestamp: performance.now(),
        codedWidth: width,
        codedHeight: height,
      });
    },
    rgb24ToVideoFrame(bytes, outFormat) {
      // packed 24bpp (RGB3/BGR3) isn't a valid VideoFrame format, expand to 32bpp by adding alpha
      const { width, height } = this.format;
      const stride = Math.floor(bytes.length / height); // row may be padded (bytesperline != width*3)
      const rgba = new Uint8ClampedArray(width * height * 4);
      let out = 0;
      for (let row = 0; row < height; row++) {
        let i = row * stride;
        for (let col = 0; col < width; col++) {
          rgba[out]     = bytes[i];
          rgba[out + 1] = bytes[i + 1];
          rgba[out + 2] = bytes[i + 2];
          rgba[out + 3] = 255;
          out += 4;
          i += 3;
        }
      }
      return new VideoFrame(rgba, {
        format: outFormat,
        timestamp: performance.now(),
        codedWidth: width,
        codedHeight: height,
      });
    },
    packed422ToVideoFrame(bytes, layout) {
      // packed 4:2:2 (YUYV/YVYU/UYVY/VYUY) isn't a valid VideoFrame format, convert to RGBA
      const { width, height } = this.format;
      const stride = Math.floor(bytes.length / height); // row may be padded (bytesperline != width*2)
      const rgba = new Uint8ClampedArray(width * height * 4);
      let out = 0;
      for (let row = 0; row < height; row++) {
        let i = row * stride;
        for (let col = 0; col < width; col += 2) {
          const y0 = bytes[i + layout.y0], u = bytes[i + layout.u], y1 = bytes[i + layout.y1], v = bytes[i + layout.v];
          out = this.writeYUVPixel(rgba, out, y0, u, v);
          out = this.writeYUVPixel(rgba, out, y1, u, v);
          i += 4;
        }
      }
      return new VideoFrame(rgba, {
        format: 'RGBA',
        timestamp: performance.now(),
        codedWidth: width,
        codedHeight: height,
      });
    },
    writeYUVPixel(out, offset, y, u, v) {
      const c = y - 16, d = u - 128, e = v - 128;
      const clamp = (n) => Math.min(255, Math.max(0, n));
      out[offset]     = clamp((298 * c + 409 * e + 128) >> 8);
      out[offset + 1] = clamp((298 * c - 100 * d - 208 * e + 128) >> 8);
      out[offset + 2] = clamp((298 * c + 516 * d + 128) >> 8);
      out[offset + 3] = 255;
      return offset + 4;
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
