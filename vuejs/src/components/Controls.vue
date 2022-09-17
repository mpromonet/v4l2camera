<template>
  <div>
    <tr v-for="c in controls" :key="c.name">
      <td class="key">{{ c.name }}</td>
      <td class="value" v-if="c.menu">
        <div>
          <v-select :items="getItems(c.menu)"
                    v-model.number="c.value"
                    v-on:change="updateValue(c.id,c.value)">
          </v-select>
        </div>
      </td>        
      <td class="value" v-if="!c.menu && c.minimum == 0 && c.maximum == 1">
        <v-switch
          v-model.number="c.value"
          color="blue"
          :true-value="1" 
          :false-value="0"
          v-on:change="updateValue(c.id,c.value)">
        </v-switch>
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
    },
    getItems: function(menu) {
      return menu.map(item => ({"title": item.label, "value": item.value}))
    }
  }
};
</script>

<style scoped>
tr {
  display: flex;
}
.key {
  width: 20%;
}
.value {
  width: 80%;
}
</style>
