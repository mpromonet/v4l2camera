<template>
  <div>
    <h1>{{ msg }}</h1>
    {{ format.description }} {{ format.width }}x{{ format.height }}x{{ format.fps }}
    <select
      v-model="format.format"
      v-on:change="updateValue(format)"
    >
      <option v-for="o in formats" :key="o.format" >{{ o.format }}</option>
    </select>
    <select v-model="geometry" v-on:change="updateGeometry()">
      <option v-for="f in format.frameSizes" :key="f" >{{f.width}}x{{f.height}}</option>
    </select>
    <select v-model="format.fps">
      <option v-for="f in format.frameSizes.filter((fmt) => fmt.width + 'x' + fmt.height == geometry)[0].intervals" :key="f" >{{f.fps}}</option>
    </select>
  </div>
</template>

<script>
import axios from "axios";

var serviceurl = "";

export default {
  mounted() {
    axios({ method: "GET", url: serviceurl + "/api/capabilities" }).then(
      (response) => {
        this.msg = response.data.card;
      }
    );
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
      msg: "loading...",
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
