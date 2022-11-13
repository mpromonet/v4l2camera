<template>
  <v-container>
    <v-row>
      <v-col>Format</v-col>
      <v-col cols="8">
        <v-select
          v-model="format.format"
          :items="getFormats(formats)"
          :hint="format.description"
          persistent-hint
          @update:modelValue="updateValue(format)" >
        </v-select>
      </v-col>
    </v-row>

    <v-row>
      <v-col>Geometry</v-col>
      <v-col cols="8" v-if="typeof format.frameSizes[0].width === 'number' && typeof format.frameSizes[0].height === 'number'">
        <v-select v-model="geometry" :items="getGeometries(format.frameSizes)" @update:modelValue="updateGeometry()">
        </v-select>
      </v-col>
      <v-col cols="7" v-if="typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object'">
          <v-slider
              v-model.number="format.width"
              :min="format.frameSizes[0].width.min"
              :max="format.frameSizes[0].width.max"
              :step="format.frameSizes[0].width.step" 
              color="blue"
              thumb-label="always"
              @update:modelValue="updateValue(format)"
            >
            <template v-slot:prepend>{{format.frameSizes[0].width.min}}</template>
            <template v-slot:append>{{format.frameSizes[0].width.max}}</template>
          </v-slider>

          <v-slider
              v-model.number="format.height"
              :min="format.frameSizes[0].height.min"
              :max="format.frameSizes[0].height.max"
              color="blue"
              thumb-label="always"
              :step="format.frameSizes[0].height.step" @change="alert($event)"
              @update:modelValue="updateValue(format)"
            >
            <template v-slot:prepend>{{format.frameSizes[0].height.min}}</template>
            <template v-slot:append>{{format.frameSizes[0].height.max}}</template>
          </v-slider> 
      </v-col>
      <v-col cols="1" v-if="typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object'">
          <v-text-field
            v-model.number="format.width"
            @update:modelValue="updateValue(format)">
          </v-text-field>
          <v-text-field
            v-model.number="format.height"
            @update:modelValue="updateValue(format)">
          </v-text-field>
      </v-col>
    </v-row>
    
    <v-row>
      <v-col>Fps</v-col>
      <v-col cols="8" v-if="typeof format.frameSizes[0].intervals[0].fps === 'number'" >
        <v-select v-model.number="format.fps" :items="getFps(format.frameSizes)">
        </v-select>
      </v-col>
      <v-col cols="7" v-if="typeof format.frameSizes[0].intervals[0].fps === 'object'">
        <v-slider 
            v-model.number="format.fps"
            :min="format.frameSizes[0].intervals[0].fps.min"
            :max="format.frameSizes[0].intervals[0].fps.max"
            color="blue"
            thumb-label="always"
            :step="format.frameSizes[0].intervals[0].fps.step" 
            @update:modelValue="updateValue(format)"
          >
            <template v-slot:prepend>{{format.frameSizes[0].intervals[0].fps.min}}</template>
            <template v-slot:append>{{format.frameSizes[0].intervals[0].fps.max}}</template>
        </v-slider>
      </v-col>
      <v-col cols="1" v-if="typeof format.frameSizes[0].intervals[0].fps === 'object'">
          <v-text-field
            v-model.number="format.fps"
            @update:modelValue="updateValue(format)">
          </v-text-field>
      </v-col>
    </v-row>
     

    <v-row v-if="format.audio">
      <v-col>Audio Format</v-col>
      <v-col cols="8">{{format.audio.format}}</v-col>
    </v-row>    
    <v-row v-if="format.audio">
      <v-col>Audio Sample Rate</v-col>
      <v-col cols="8">{{format.audio.samplerate}}</v-col>
    </v-row>    
    <v-row v-if="format.audio">
      <v-col>Audio Channels</v-col>
      <v-col cols="8">{{format.audio.channels}}</v-col>
    </v-row>    
  </v-container>
</template>

<script>
import axios from 'axios';
import config from '../config.js';

export default {
  mounted() {
    axios.get(config.serviceurl + "/api/formats").then(
      (response) => {
        this.formats = response.data;

        axios.get(config.serviceurl + "/api/format").then(
          (response) => {
            this.format = response.data;
            this.updateFormatFields();
          }
        );
      }
    );
  },
  data() {
    return {
      formats: [],
      format: "",
      geometry: ""
    };
  },
  methods: {
    updateValue(format) {
      axios.post(config.serviceurl + "/api/format", format ).then(
        (response) => {
          this.format = response.data;
          this.updateFormatFields();
        }
		  );
	  },
    updateGeometry() {
        console.log(this.geometry)
        const sizes = this.geometry.split('x');
        this.format.width = parseInt(sizes[0]);
        this.format.height = parseInt(sizes[1]);
        this.updateValue(this.format);
	  },
    updateFormatFields() {
        this.geometry = `${this.format.width}x${this.format.height}`;
        this.format.description = this.formats.filter((d) => d.format == this.format.format)[0].description;
        this.format.frameSizes = this.formats.filter((d) => d.format == this.format.format)[0].frameSizes;
    },
    getFormats(formats) {
        return formats.map(item => item.format);
    },
    getGeometries(formats) {
        return formats.sort((a,b) => a.width*a.height - b.width*b.height).map(item => `${item.width}x${item.height}`);
    },
    getFps(formats) {
        return formats.filter((fmt) => fmt.width + 'x' + fmt.height == this.geometry)[0].intervals.map(f => f.fps);
    }
  }
};
</script>

<style scoped>
</style>
