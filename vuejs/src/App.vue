<template>
  <v-app>
    <!-- App Bar -->
    <v-app-bar elevation="0" color="surface" border="b">
      <template v-slot:prepend>
        <v-icon class="ml-2 mr-1" color="primary" size="small">mdi-camera</v-icon>
        <span class="font-weight-bold" style="font-size:1rem;">{{ msg }}</span>
      </template>
      <v-app-bar-title />
      <template v-slot:append>
        <v-btn
          :icon="showParams ? 'mdi-cog' : 'mdi-cog-outline'"
          variant="text"
          size="small"
          :color="showParams ? 'primary' : 'grey'"
          @click="showParams = !showParams"
        />
      </template>
    </v-app-bar>

    <v-main>
      <v-container fluid class="pa-4" style="max-width:1400px;">
        <v-row>

          <!-- Camera view -->
          <v-col cols="12" :lg="showParams ? 6 : 12">
            <Video />
          </v-col>

          <!-- Parameters panel -->
          <v-col v-if="showParams" cols="12" lg="6">
            <div class="params-panel">
              <v-expansion-panels variant="accordion" multiple v-model="openPanels">

                <v-expansion-panel value="rtsp" bg-color="surface" elevation="2" rounded="lg" class="mb-2">
                  <v-expansion-panel-title class="text-body-2 font-weight-medium">
                    <v-icon class="mr-2" size="small" color="primary">mdi-access-point-network</v-icon>
                    RTSP
                  </v-expansion-panel-title>
                  <v-expansion-panel-text>
                    <Rtsp />
                  </v-expansion-panel-text>
                </v-expansion-panel>

                <v-expansion-panel value="formats" bg-color="surface" elevation="2" rounded="lg" class="mb-2">
                  <v-expansion-panel-title class="text-body-2 font-weight-medium">
                    <v-icon class="mr-2" size="small" color="primary">mdi-film</v-icon>
                    Formats
                  </v-expansion-panel-title>
                  <v-expansion-panel-text>
                    <Format />
                  </v-expansion-panel-text>
                </v-expansion-panel>

                <v-expansion-panel value="controls" bg-color="surface" elevation="2" rounded="lg">
                  <v-expansion-panel-title class="text-body-2 font-weight-medium">
                    <v-icon class="mr-2" size="small" color="primary">mdi-tune</v-icon>
                    Controls
                  </v-expansion-panel-title>
                  <v-expansion-panel-text>
                    <Controls />
                  </v-expansion-panel-text>
                </v-expansion-panel>

              </v-expansion-panels>
            </div>
          </v-col>

        </v-row>
      </v-container>
    </v-main>

    <!-- Footer -->
    <v-footer app color="surface" border="t" height="36" class="px-4">
      <v-spacer />
      <v-btn
        variant="text"
        size="x-small"
        density="compact"
        href="https://github.com/mpromonet/v4l2camera"
        target="_blank"
        prepend-icon="mdi-github"
      >v4l2camera</v-btn>
      <v-chip
        v-if="version"
        color="primary"
        variant="tonal"
        size="x-small"
        class="ml-3 font-weight-medium"
        prepend-icon="mdi-tag-outline"
      >{{ version }}</v-chip>
      <v-spacer />
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
  components: { Format, Controls, Video, Rtsp },
  mounted() {
    axios.get("/api/capabilities").then(
      (response) => {
        this.msg = response.data.card;
        document.title = this.msg;
      }
    );
    axios.get("/api/version").then(
      (response) => this.version = response.data.replace(/-g[0-9a-f]+[^/]*/i, '')
    );
  },
  data() {
    return {
      msg: "v4l2camera",
      version: "",
      showParams: true,
      openPanels: ["rtsp", "formats", "controls"],
    };
  }
}
</script>

<style>
html, body {
  overflow-y: auto;
  background-color: #0F1923;
}

/* On large screens the params column sticks under the app bar */
@media (min-width: 1280px) {
  .params-panel {
    position: sticky;
    top: 64px;          /* height of v-app-bar */
    max-height: calc(100vh - 64px - 36px);  /* minus app-bar and footer */
    overflow-y: auto;
  }
}
</style>
