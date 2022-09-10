<template>
  <div>
    <tr>
      {{ format.description }} {{ format.width }}x{{ format.height }}x{{ format.fps }}
    </tr>
    <tr>
      <td>Format:</td>
      <td>
        <select
          v-model="format.format"
          v-on:change="updateValue(format)"
        ><option v-for="o in formats" :key="o.format" >{{ o.format }}</option></select>
      </td>
    </tr>
    <tr>
      <td>Geometry:</td>
      <td v-if="typeof format.frameSizes[0].width === 'number' && typeof format.frameSizes[0].height === 'number'">
        <select v-model="geometry" v-on:change="updateGeometry()">
          <option v-for="f in format.frameSizes" :key="f" >{{f.width}}x{{f.height}}</option>
        </select>
      </td>
      <td v-if="typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object'">
          <v-slider
              v-model.number="format.width"
              :min="format.frameSizes[0].width.min"
              :max="format.frameSizes[0].width.max"
              :step="format.frameSizes[0].width.step" 
              color="blue"
              thumb-label="always"
              v-on:change="updateValue(format)"
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
              :step="format.frameSizes[0].height.step" 
              v-on:change="updateValue(format)"
            >
            <template v-slot:prepend>{{format.frameSizes[0].height.min}}</template>
            <template v-slot:append>{{format.frameSizes[0].height.max}}</template>
          </v-slider> 
      </td>
    </tr>
    <tr>
      <td>Fps:</td>
      <td v-if="typeof format.frameSizes[0].fps === 'object'" >
        <select v-model="format.fps">
          <option v-for="f in format.frameSizes.filter((fmt) => fmt.width + 'x' + fmt.height == geometry)[0].intervals" :key="f" >{{f.fps}}</option>
        </select>
      </td>
      <td v-if="typeof format.frameSizes[0].intervals[0].fps === 'object'">
        <v-slider 
            v-model.number="format.fps"
            :min="format.frameSizes[0].intervals[0].fps.min"
            :max="format.frameSizes[0].intervals[0].fps.max"
            color="blue"
            thumb-label="always"
            :step="format.frameSizes[0].intervals[0].fps.step" 
            v-on:change="updateValue(format)"
          >
            <template v-slot:prepend>{{format.frameSizes[0].intervals[0].fps.min}}</template>
            <template v-slot:append>{{format.frameSizes[0].intervals[0].fps.max}}</template>
        </v-slider>
      </td>
    </tr>
     
  </div>
</template>

<script>
import axios from "axios";

var serviceurl = "";

export default {
  mounted() {
    axios({ method: "GET", url: serviceurl + "/api/formats" }).then(
      (response) => {
        this.formats = response.data;

        axios({ method: "GET", url: serviceurl + "/api/format" }).then(
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
      axios.post(serviceurl + "/api/format", format ).then(
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
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
