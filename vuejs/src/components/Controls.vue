<template>
  <div>
    <tr v-for="c in controls" :key="c.name">
      <td>{{ c.name }}</td>
      <td>
		  <input v-model="c.value" type="text" />
	  </td>
      <td>[ {{ c.minimum }} ,</td>
      <td>{{ c.maximum }} ]</td>
      <td>
        <select v-if="c.menu" v-model="c.value" v-on:change="updateValue(c.id,parseInt(c.value))">
          <option v-for="m in c.menu" :key="m.label" :value="m.value">{{ m.label }}</option>
        </select>
        <input
          v-if="!c.menu && c.minimum == 0 && c.maximum == 1"
          v-model="c.value" true-value="1" false-value="0"
          type="checkbox" v-on:change="updateValue(c.id,parseInt(c.value))"
        />
        <input
          v-if="!c.menu && !(c.minimum == 0 && c.maximum == 1)"
          v-model="c.value"
          :min="c.minimum"
          :max="c.maximum"
          type="range"
          :step="c.step" v-on:change="updateValue(c.id,parseInt(c.value))"
        />
      </td>
    </tr>
  </div>
</template>

<script>
import axios from "axios";

var serviceurl = "";

export default {
  mounted() {
    axios.get(serviceurl + "/api/controls").then(
      (response) => {
        this.controls = response.data;
      }
    );
  },
  data() {
    return {
      controls: [],
    };
  },
  methods: {
    updateValue: function(id, value) {
		axios.post(serviceurl + "/api/control", {id, value} ).then(
			(response) => {
				this.controls.filter((d) => d.id == id)[0].value = response.data.value;
			}
		);
	}
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
