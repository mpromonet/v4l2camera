<template>
  <div id="app">
    <img src="./assets/logo.png">
    <Format/>
    <Controls/>
  </div>
</template>

<script>
import Format from './components/Format'
import Controls from './components/Controls'

export default {
  name: 'App',
  components: {
    Format,
    Controls
  }
}
</script>

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
}
</style>
