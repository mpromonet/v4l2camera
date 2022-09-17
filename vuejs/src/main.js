import 'vuetify/styles'
import { createApp } from 'vue'
import { createVuetify } from 'vuetify'
import App from './App.vue'

const vuetify = createVuetify();

createApp(App)
  .use(vuetify)
  .mount('#app')
