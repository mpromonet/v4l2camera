<template>
  <v-container class="pa-2">

    <!-- Format -->
    <v-row align="center" dense class="mb-2">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">Format</v-col>
      <v-col cols="9">
        <v-select
          v-model="format.format"
          :items="getFormats(formats)"
          :hint="format.description"
          persistent-hint
          density="compact"
          variant="outlined"
          @update:modelValue="updateValue(format)"
        />
      </v-col>
    </v-row>

    <!-- Geometry -->
    <v-row align="center" dense class="mb-2">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">Geometry</v-col>

      <!-- Fixed sizes → dropdown -->
      <v-col cols="9" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].width === 'number' && typeof format.frameSizes[0].height === 'number'">
        <v-select
          v-model="geometry"
          :items="getGeometries(format.frameSizes)"
          density="compact"
          variant="outlined"
          hide-details
          @update:modelValue="updateGeometry()"
        />
      </v-col>

      <!-- Continuous sizes → sliders -->
      <v-col cols="9" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object'">
        <v-slider
          v-model.number="format.width"
          :min="format.frameSizes[0].width.min"
          :max="format.frameSizes[0].width.max"
          :step="format.frameSizes[0].width.step"
          label="W"
          color="primary"
          density="compact"
          thumb-label
          hide-details
          @end="updateValue(format)"
        />
        <v-slider
          v-model.number="format.height"
          :min="format.frameSizes[0].height.min"
          :max="format.frameSizes[0].height.max"
          :step="format.frameSizes[0].height.step"
          label="H"
          color="primary"
          density="compact"
          thumb-label
          hide-details
          @end="updateValue(format)"
        />
      </v-col>

      <!-- Manual input fallback -->
      <v-col cols="4" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object' || !format.frameSizes || !format.frameSizes.length">
        <v-text-field label="Width" v-model.number="format.width" density="compact" variant="outlined" hide-details @update:modelValue="updateValue(format)" />
      </v-col>
      <v-col cols="4" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].width === 'object' && typeof format.frameSizes[0].height === 'object' || !format.frameSizes || !format.frameSizes.length">
        <v-text-field label="Height" v-model.number="format.height" density="compact" variant="outlined" hide-details @update:modelValue="updateValue(format)" />
      </v-col>
    </v-row>

    <!-- FPS -->
    <v-row align="center" dense class="mb-2">
      <v-col cols="3" class="text-body-2 text-medium-emphasis">FPS</v-col>

      <v-col cols="9" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].intervals[0].fps === 'number'">
        <v-select
          v-model.number="format.fps"
          :items="getFps(format.frameSizes)"
          density="compact"
          variant="outlined"
          hide-details
        />
      </v-col>

      <v-col cols="9" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].intervals[0].fps === 'object'">
        <v-slider
          v-model.number="format.fps"
          :min="format.frameSizes[0].intervals[0].fps.min"
          :max="format.frameSizes[0].intervals[0].fps.max"
          :step="format.frameSizes[0].intervals[0].fps.step"
          label="FPS"
          color="primary"
          density="compact"
          thumb-label
          hide-details
          @end="updateValue(format)"
        />
      </v-col>

      <v-col cols="4" v-if="format.frameSizes && format.frameSizes[0] && typeof format.frameSizes[0].intervals[0].fps === 'object' || !format.frameSizes || !format.frameSizes.length">
        <v-text-field label="FPS" v-model.number="format.fps" density="compact" variant="outlined" hide-details @update:modelValue="updateValue(format)" />
      </v-col>
    </v-row>

    <!-- Audio -->
    <template v-if="format.audio">
      <v-divider class="my-3" />
      <v-row align="center" dense class="mb-2">
        <v-col cols="3" class="text-body-2 text-medium-emphasis">Audio Format</v-col>
        <v-col cols="9">
          <v-select
            v-model="format.audio.format"
            :items="audioFormats"
            density="compact"
            variant="outlined"
            hide-details
            @update:modelValue="updateValue(format)"
          />
        </v-col>
      </v-row>
      <v-row align="center" dense class="mb-2">
        <v-col cols="3" class="text-body-2 text-medium-emphasis">Sample Rate</v-col>
        <v-col cols="9" class="text-body-2">{{ format.audio.samplerate }}</v-col>
      </v-row>
      <v-row align="center" dense>
        <v-col cols="3" class="text-body-2 text-medium-emphasis">Channels</v-col>
        <v-col cols="9" class="text-body-2">{{ format.audio.channels }}</v-col>
      </v-row>
    </template>

  </v-container>
</template>

<script>
import axios from 'axios';

export default {
  mounted() {
    axios.get("/api/formats").then(
      (response) => {
        this.formats = response.data.video;
        this.audioFormats = response.data.audio;

        axios.get("/api/format").then(
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
      audioFormats: [],
      geometry: ""
    };
  },
  methods: {
    updateValue(format) {
      axios.post("/api/format", format ).then(
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
