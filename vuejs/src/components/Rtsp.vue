<template>
  <div>     
    <v-container>
      <v-row align="center" justify="center">
          {{ this.rtspinfo.url }}
      </v-row>
    </v-container>      
    <v-container>
      <v-row>
        <v-col>RTSP Uri</v-col>
        <v-col cols="8">
          <v-text-field v-model="rtspinfo.rtspuri" @update:modelValue="updateRtspInfo()">
          </v-text-field>
        </v-col>
      </v-row>
      <v-row>
        <v-col>Multicast Uri</v-col>
        <v-col cols="8">
          <v-text-field v-model="rtspinfo.multicasturi" @update:modelValue="updateRtspInfo()">
          </v-text-field>
        </v-col>
      </v-row>
      <v-row>
        <v-col>SRTP</v-col>
        <v-col cols="8">
          <v-switch
            v-model.number="rtspinfo.issrtp" @update:modelValue="updateRtspInfo()"
            color="blue">
          </v-switch>      
        </v-col>
      </v-row>
      <v-row>
        <v-col>SRTP encrypted</v-col>
        <v-col cols="8">
          <v-switch
            v-model.number="rtspinfo.issrtpencrypted" @update:modelValue="updateRtspInfo()"
            color="blue">
          </v-switch>      
        </v-col>
      </v-row>
      <v-row>
        <v-col>RTSPS</v-col>
        <v-col cols="8">
          <v-switch
            v-model.number="rtspinfo.issrtsps" @update:modelValue="updateRtspInfo()"
            color="blue">
          </v-switch>      
        </v-col>
      </v-row>
    </v-container>
  </div>
</template>

<script>
import axios from "axios";

export default {
  data() {
    return {
      rtspinfo: {rtspuri: '', multicasturi: '', url: '', issrtp: false, issrtpencrypted: false, issrtsps: false}
    };
  },
  mounted() {
    axios.get("/api/rtspinfo").then(
      (response) => this.rtspinfo = response.data
    );
  },
  methods: {
    async updateRtspInfo() {
        const response = await axios.post("/api/rtspinfo", this.rtspinfo);
        this.rtspinfo = response.data;      
    }
  }
};
</script>

<style>
</style>
