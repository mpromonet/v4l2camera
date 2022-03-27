<template>
  <div id="app">
    <Format/>
    <Video/>
    <Controls/>
  </div>
</template>

<script>
import Video from './components/Video.vue'
import Format from './components/Format.vue'
import Controls from './components/Controls.vue'

export default {
  name: 'App',
  components: {
    Format,
    Controls,
    Video
  }
}
</script>

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
}
</style>
