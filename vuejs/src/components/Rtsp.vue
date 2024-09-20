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
          <v-text-field v-model="debouncedRtspUri" @keydown="handleKeydown('rtspuri', $event)" :class="{ 'debouncing': isDebouncingRtspUri }" >
          </v-text-field>
        </v-col>
      </v-row>
      <v-row>
        <v-col>Multicast Uri</v-col>
        <v-col cols="8">
          <v-text-field v-model="debouncedMulticastUri" @keydown="handleKeydown('multicasturi', $event)" >
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
      get() {
        return this.rtspinfo.multicasturi;
      },
      set: debounce(function (value) {
        this.rtspinfo.multicasturi = value;
        this.updateRtspInfo();
      }, 1000) 
    },
    debouncedRtspUri: {
      get() {
        return this.rtspinfo.rtspuri;
      },
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
    debouncedSetRtspUri: debounce(function (value) {
      this.updateRtspInfo();
    }, 1000)      
  }
};
</script>

<style>
.debouncing {
  color: red; 
}
</style>
