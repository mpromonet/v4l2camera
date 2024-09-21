<template>
    <v-app>
      <h1>{{ msg }}</h1>
      <v-divider></v-divider>
      <v-main id="content">
        <Video/>
        <v-divider></v-divider>
        <h3 @click="isRtspVisible = !isRtspVisible">RTSP</h3>
        <Rtsp v-if="isRtspVisible"/>
        <h3 @click="isFormatVisible = !isFormatVisible">Formats</h3>
        <Format v-if="isFormatVisible"/>
        <h3 @click="isControlsVisible = !isControlsVisible">Controls</h3>
        <Controls v-if="isControlsVisible"/>
      </v-main>
      <v-divider></v-divider>
      <v-footer>
        <v-container>
          <v-row align="center" justify="center">
                <v-icon>mdi-github</v-icon><a href="https://github.com/mpromonet/v4l2camera">v4l2camera</a><p>{{ version }}</p>
          </v-row>
       </v-container>
      </v-footer>
    </v-app>
</template>

<script>
import Video from './components/Video.vue';
import Rtsp from './components/Rtsp.vue';
import Format from './components/Format.vue';
import Controls from './components/Controls.vue';
import axios from "axios";
axios.defaults.baseURL = import.meta.env.VITE_APP_BASE_URL

export default {
  name: 'App',
  components: {
    Format,
    Controls,
    Video,
    Rtsp
  },
  mounted() {
    axios.get("/api/capabilities").then(
      (response) =>  {
        this.msg = response.data.card
        document.title = this.msg;
      } 
    );
    axios.get("/api/version").then(
      (response) => this.version = response.data); 
  },
  data() {
    return {
      msg: "...",
      version: "",
      isRtspVisible: true,
      isFormatVisible: true,
      isControlsVisible: true
    };
  }
}
</script>

<style>
html {
  overflow-y: hidden;
}
#content {
  height: calc(100vh - 100px);
  overflow-y: auto;
}
h1 {
  text-align: center;
}
h3 {
  text-align: center;
}
p {
  margin-left: 0.5em;
}
</style>
