<template>
  <v-container>
    <v-row v-for="c in controls" :key="c.name">
      <v-col>{{ c.name }}</v-col>
      <v-col cols="8" v-if="c.menu">
          <v-select :items="getItems(c.menu)"
                    v-model.number="c.value"
                    @update:modelValue="updateValue(c.id,c.value)">
          </v-select>
      </v-col>        
      <v-col cols="8" v-if="!c.menu && c.minimum == 0 && c.maximum == 1">
        <v-switch
          v-model.number="c.value"
          color="blue"
          :true-value="1" 
          :false-value="0"
          @update:modelValue="updateValue(c.id,c.value)">
        </v-switch>
      </v-col>        
      <v-col cols="7" v-if="!c.menu && !(c.minimum == 0 && c.maximum == 1)">
        <v-slider
          v-model.number="c.value" 
          color="blue"
          hide-spin-buttons
          :min="c.minimum"
          :max="c.maximum" 
          :step="c.step"
          ticks="always"
          :disabled="disabled(c.flags)"
          @update:modelValue="updateValue(c.id,c.value)"
        >
          <template v-slot:prepend>{{c.minimum}}</template>
          <template v-slot:append>{{c.maximum}}</template>
        </v-slider>
      </v-col>
      <v-col cols="1" v-if="!c.menu && !(c.minimum == 0 && c.maximum == 1)">
          <v-text-field
            v-model.number="c.value"
            @update:modelValue="updateValue(c.id,c.value)">
          </v-text-field>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import axios from "axios";
import config from '../config.js';

export default {
  mounted() {
    axios.get(config.serviceurl + "/api/controls").then(
      (response) => this.controls = response.data
    );
  },
  data() {
    return {
      controls: [],
    };
  },
  methods: {
    updateValue(id, value) {
      axios.post(config.serviceurl + "/api/control", {id, value} ).then(
        (response) => this.controls.filter((d) => d.id == id).forEach((element) => element.value = response.data.value)
      );
    },
    getItems(menu) {
      return menu.map(item => ({"title": item.label, "value": item.value}));
    },
    disabled(flags) {
      return (flags.find( (f) => f === "V4L2_CTRL_FLAG_INACTIVE") !== undefined);
    }
  }
};
</script>

<style scoped>
</style>
