<template>
  <v-container class="pa-2">

    <!-- Live URL banner -->
    <v-row v-if="rtspinfo.url" class="mb-3">
      <v-col>
        <v-alert
          type="info"
          variant="tonal"
          density="compact"
          icon="mdi-broadcast"
          :text="rtspinfo.url"
        />
      </v-col>
    </v-row>

    <!-- RTSP URI -->
    <v-row align="center" dense class="mb-2">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">RTSP URI</v-col>
      <v-col cols="9">
        <v-text-field
          v-model="debouncedRtspUri"
          density="compact"
          variant="outlined"
          hide-details
          :class="{ 'debouncing': isDebouncingRtspUri }"
          @keydown="handleKeydown('rtspuri', $event)"
        />
      </v-col>
    </v-row>

    <!-- Multicast URI -->
    <v-row align="center" dense class="mb-2">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">Multicast URI</v-col>
      <v-col cols="9">
        <v-text-field
          v-model="debouncedMulticastUri"
          density="compact"
          variant="outlined"
          hide-details
          @keydown="handleKeydown('multicasturi', $event)"
        />
      </v-col>
    </v-row>

    <v-divider class="my-3" />

    <!-- Toggles -->
    <v-row align="center" dense class="mb-1">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">SRTP</v-col>
      <v-col cols="9">
        <v-switch
          v-model.number="rtspinfo.issrtp"
          color="primary"
          density="compact"
          hide-details
          @update:modelValue="updateRtspInfo()"
        />
      </v-col>
    </v-row>
    <v-row align="center" dense class="mb-1">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">SRTP Encrypted</v-col>
      <v-col cols="9">
        <v-switch
          v-model.number="rtspinfo.issrtpencrypted"
          color="primary"
          density="compact"
          hide-details
          @update:modelValue="updateRtspInfo()"
        />
      </v-col>
    </v-row>
    <v-row align="center" dense>
      <v-col cols="3" class="text-body-2 text-medium-emphasis">RTSPS</v-col>
      <v-col cols="9">
        <v-switch
          v-model.number="rtspinfo.issrtsps"
          color="primary"
          density="compact"
          hide-details
          @update:modelValue="updateRtspInfo()"
        />
      </v-col>
    </v-row>

  </v-container>
</template>

<script>
import axios from "axios";
import { debounce } from 'lodash';

export default {
  data() {
    return {
      rtspinfo: {rtspuri: '', multicasturi: '', url: '', issrtp: false, issrtpencrypted: false, issrtsps: false},
      isDebouncingRtspUri: false
    };
  },
  mounted() {
    axios.get("/api/rtspinfo").then(
      (response) => this.rtspinfo = response.data
    );
  },
  computed: {
    debouncedMulticastUri: {
      get() { return this.rtspinfo.multicasturi; },
      set: debounce(function (value) {
        this.rtspinfo.multicasturi = value;
        this.updateRtspInfo();
      }, 1000)
    },
    debouncedRtspUri: {
      get() { return this.rtspinfo.rtspuri; },
      set(value) {
        this.isDebouncingRtspUri = true;
        this.rtspinfo.rtspuri = value;
        this.debouncedSetRtspUri(value);
      }
    }
  },
  methods: {
    async updateRtspInfo() {
      const response = await axios.post("/api/rtspinfo", this.rtspinfo);
      this.rtspinfo = response.data;
      this.isDebouncingRtspUri = false;
    },
    handleKeydown(field, event) {
      if (event.key === 'Enter') {
        this.rtspinfo[field] = event.target.value;
        this.updateRtspInfo();
      }
    },
    debouncedSetRtspUri: debounce(function () {
      this.updateRtspInfo();
    }, 1000)
  }
};
</script>

<style scoped>
.debouncing :deep(input) {
  color: rgb(var(--v-theme-warning)) !important;
}
</style>
