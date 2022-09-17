<template>
  <div id="app">
    <h1>{{ msg }}</h1>
    <Video/>
    <Format/>
    <Controls/>
  </div>
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
    axios({ method: "GET", url: config.serviceurl + "/api/capabilities" }).then(
      (response) => this.msg = response.data.card
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
