<template>
  <v-app>
    <h1>{{ msg }}</h1>
    <v-divider></v-divider>
    <Video/>
    <v-container>
      <v-row>
        <v-expansion-panels>
          <v-expansion-panel>
              <v-expansion-panel-header>Format</v-expansion-panel-header>
              <v-expansion-panel-content><Format/></v-expansion-panel-content>
          </v-expansion-panel>
          <v-expansion-panel>
              <v-expansion-panel-header>Controls</v-expansion-panel-header>
              <v-expansion-panel-content><Controls/></v-expansion-panel-content>
          </v-expansion-panel>
        </v-expansion-panels>  
      </v-row>
    </v-container>
    <v-divider></v-divider>
    <v-footer>
      <v-container>
        <v-row align="center" justify="center">
              <v-icon>mdi-github</v-icon><a href="https://github.com/mpromonet/v4l2camera">v4l2camera</a>
        </v-row>
     </v-container>
    </v-footer>
  </v-app>
</template>

<script>
import Video from './components/Video.vue';
import Format from './components/Format.vue';
import Controls from './components/Controls.vue';
import axios from "axios";
import config from './config.js';

export default {
  name: 'App',
  components: {
    Format,
    Controls,
    Video
  },
  mounted() {
    axios.get(config.serviceurl + "/api/capabilities").then(
      (response) =>  {
        this.msg = response.data.card
        document.title = this.msg;
      } 
    );
  },
  data() {
    return {
      msg: "..."
    };
  }
}
</script>

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
}
h1 {
  text-align: center;
}
</style>
