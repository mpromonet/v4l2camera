<template>
  <div>
    <tr v-for="c in controls" :key="c.name">
      <td class="key">{{ c.name }}</td>
      <td class="value" v-if="c.menu">
        <select v-model.number="c.value" v-on:change="updateValue(c.id,c.value)">
          <option v-for="m in c.menu" :key="m.label" :value="m.value">{{ m.label }}</option>
        </select>
      </td>        
      <td class="value" v-if="!c.menu && c.minimum == 0 && c.maximum == 1">
        <input
          v-model.number="c.value" true-value="1" false-value="0"
          type="checkbox" 
          v-on:change="updateValue(c.id,c.value)"
        />
      </td>        
      <td class="value" v-if="!c.menu && !(c.minimum == 0 && c.maximum == 1)">
        <v-slider
          v-model.number="c.value" 
          color="blue"
          thumb-label="true"          
          :min="c.minimum"
          :max="c.maximum" 
          v-on:change="updateValue(c.id,c.value)"
        >
          <template v-slot:prepend>{{c.minimum}}</template>
          <template v-slot:append>{{c.maximum}}
                <v-text-field
                  v-model.number="c.value"
                  v-on:change="updateValue(c.id,c.value)"
                ></v-text-field>
          </template>
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
    axios.get(serviceurl + "/api/controls").then(
      (response) => this.controls = response.data
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
        (response) => this.controls.filter((d) => d.id == id)[0].value = response.data.value
      );
    }
  }
};
</script>

<style scoped>
tr {
  width: 100%;
}
.key {
  width: 20%;
}
.value {
  width: 80%;
}
</style>
