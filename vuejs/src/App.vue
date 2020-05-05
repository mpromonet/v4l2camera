<template>
  <div id="app">
    <Video/>
    <Format/>
    <Controls/>
  </div>
</template>

<script>
import Video from './components/Video'
import Format from './components/Format'
import Controls from './components/Controls'

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
