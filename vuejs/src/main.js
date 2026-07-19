import 'vuetify/styles'
import { createApp } from 'vue'
import { createVuetify } from 'vuetify'
import App from './App.vue'
import '@mdi/font/css/materialdesignicons.css'

const vuetify = createVuetify({
  theme: {
    defaultTheme: 'dark',
    themes: {
      dark: {
        dark: true,
        colors: {
          primary:    '#29B6F6',
          secondary:  '#37474F',
          surface:    '#1A2332',
          background: '#0F1923',
          error:      '#EF5350',
          success:    '#66BB6A',
        },
      },
    },
  },
});

createApp(App)
  .use(vuetify)
  .mount('#app')
