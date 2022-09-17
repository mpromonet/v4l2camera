<template>
  <v-container>
    <v-row>
      <v-col>Format:</v-col>
      <v-col cols="8">
        <v-select
          v-model="format.format"
          :items="getFormats(formats)"
          :hint="format.description"
          @update:modelValue="updateValue(format)" >
        </v-select>
      </v-col>
    </v-row>

    <v-row>
      <v-col>Geometry:</v-col>
      <v-col cols="8" v-if="typeof format.frameSizes[0].width === 'number' && typeof format.frameSizes[0].height === 'number'">
        <select v-model="geometry" v-on:change="updateGeometry()">
          <option v-for="f in format.frameSizes" :key="f" >{{f.width}}x{{f.height}}</option>
        </select>
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
      <v-col>Fps:</v-col>
      <v-col cols="8" v-if="typeof format.frameSizes[0].fps === 'object'" >
        <select v-model.number="format.fps">
          <option v-for="f in format.frameSizes.filter((fmt) => fmt.width + 'x' + fmt.height == geometry)[0].intervals" :key="f" >{{f.fps}}</option>
        </select>
      </v-col>
      <v-col cols="7" v-if="typeof format.frameSizes[0].intervals[0].fps === 'object'">
        <v-slider 
            v-model.number="format.fps"
            :min="format.frameSizes[0].intervals[0].fps.min"
            :max="format.frameSizes[0].intervals[0].fps.max"
            color="blue"
            thumb-label="true"
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
     
  </v-container>
</template>

<script>
import axios from 'axios';
import config from '../config.js';

export default {
  mounted() {
    axios({ method: "GET", url: config.serviceurl + "/api/formats" }).then(
      (response) => {
        this.formats = response.data;

        axios({ method: "GET", url: config.serviceurl + "/api/format" }).then(
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
    updateValue: function(format) {
      axios.post(config.serviceurl + "/api/format", format ).then(
        (response) => {
          this.format = response.data;
          this.updateFormatFields();
        }
		  );
	  },
    updateGeometry: function() {
      console.log(this.geometry)
      const sizes = this.geometry.split('x');
      this.format.width = parseInt(sizes[0]);
      this.format.height = parseInt(sizes[1]);
      this.updateValue(this.format);
	  },
    updateFormatFields: function() {
        this.geometry = `${this.format.width}x${this.format.height}`;
        this.format.description = this.formats.filter((d) => d.format == this.format.format)[0].description;
        this.format.frameSizes = this.formats.filter((d) => d.format == this.format.format)[0].frameSizes;
    },
    getFormats: function(formats) {
        return formats.map(item => item.format);
    }
  }
};
</script>

<style scoped>
</style>
