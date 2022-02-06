<template>
  <div>
    <h1>{{ msg }}</h1>
    {{ format.description }} {{ format.width }}x{{ format.height }}x{{ format.fps }}
    <select
      v-model="format.format"
      v-on:change="updateValue(format)"
    >
      <option v-for="o in formats" :key="o.format" :selected="o.format === format.format">
        {{ o.format }}
      </option>
    </select>

    <select>
      <option v-for="f in format.frameSizes" :selected="f.width === format.width && f.height === format.height">
        {{ f.width }}x{{ f.height }}
      </option>
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
      format: ""
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
    updateFormatFields: function() {
        this.format.description = this.formats.filter((d) => d.format == this.format.format)[0].description;
        this.format.frameSizes = this.formats.filter((d) => d.format == this.format.format)[0].frameSizes;
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
