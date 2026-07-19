<template>
  <v-container class="pa-2">
    <!-- Reset All -->
    <v-row justify="end" class="mb-2">
      <v-col cols="auto">
        <v-btn
          color="warning"
          variant="tonal"
          size="small"
          density="compact"
          prepend-icon="mdi-restore"
          :loading="resetting"
          @click="resetAll"
        >Reset All</v-btn>
      </v-col>
    </v-row>

    <v-row
      v-for="c in controls"
      :key="c.name"
      align="center"
      dense
      class="mb-1"
    >
      <!-- Label -->
      <v-col cols="3" class="text-body-2 text-medium-emphasis">{{ c.name }}</v-col>

      <!-- Menu (select) -->
      <v-col cols="7" v-if="c.menu">
        <v-select
          density="compact"
          variant="outlined"
          hide-details
          :items="getItems(c.menu)"
          v-model.number="c.value"
          @update:modelValue="updateValue(c.id, c.value)"
        />
      </v-col>
      <v-col cols="2" v-if="c.menu" class="d-flex justify-center">
        <v-btn
          icon="mdi-restore"
          variant="text"
          size="small"
          density="compact"
          color="primary"
          @click="updateValue(c.id, c.default_value)"
        />
      </v-col>

      <!-- Boolean (switch) -->
      <v-col cols="5" v-if="!c.menu && c.minimum === 0 && c.maximum === 1" class="d-flex align-center">
        <v-switch
          v-model.number="c.value"
          color="primary"
          :true-value="1"
          :false-value="0"
          density="compact"
          hide-details
          @update:modelValue="updateValue(c.id, c.value)"
        />
      </v-col>
      <v-col cols="2" v-if="!c.menu && c.minimum === 0 && c.maximum === 1" class="d-flex justify-center">
        <v-btn
          icon="mdi-restore"
          variant="text"
          size="small"
          density="compact"
          color="primary"
          @click="updateValue(c.id, c.default_value)"
        />
      </v-col>

      <!-- Slider + text field -->
      <v-col cols="5" v-if="!c.menu && !(c.minimum === 0 && c.maximum === 1)">
        <v-slider
          v-model.number="c.value"
          color="primary"
          track-color="secondary"
          hide-details
          :min="c.minimum"
          :max="c.maximum"
          :step="c.step"
          density="compact"
          thumb-label
          :disabled="disabled(c.flags)"
          @end="updateValue(c.id, c.value)"
        />
      </v-col>
      <v-col cols="2" v-if="!c.menu && !(c.minimum === 0 && c.maximum === 1)">
        <v-text-field
          v-model.number="c.value"
          variant="outlined"
          density="compact"
          hide-details
          @update:modelValue="updateValue(c.id, c.value)"
        />
      </v-col>
      <v-col cols="1" v-if="!c.menu && !(c.minimum === 0 && c.maximum === 1)" class="d-flex justify-center">
        <v-btn
          icon="mdi-restore"
          variant="text"
          size="small"
          density="compact"
          color="primary"
          @click="updateValue(c.id, c.default_value)"
        />
      </v-col>

    </v-row>
  </v-container>
</template>

<script>
import axios from "axios";

export default {
  mounted() {
    this.updateAll();
  },
  data() {
    return {
      controls: [],
      resetting: false,
    };
  },
  methods: {
    updateAll() {
      axios.get("/api/controls").then(
        (response) => this.controls = response.data
      );
    },
    updateValue(id, value) {
      axios.post("/api/control", {id, value}).then(
        (response) => this.controls.filter((d) => d.id === id).forEach((el) => el.value = response.data.value)
      ).finally(() => this.updateAll());
    },
    getItems(menu) {
      return menu.map(item => ({"title": item.label, "value": item.value}));
    },
    async resetAll() {
      this.resetting = true;
      await Promise.all(
        this.controls.map((c) => axios.post("/api/control", {id: c.id, value: c.default_value}))
      );
      await this.updateAll();
      this.resetting = false;
    },
    disabled(flags) {
      return (flags.find((f) => f === "V4L2_CTRL_FLAG_INACTIVE") !== undefined);
    },
  }
};
</script>

<style scoped>
</style>
