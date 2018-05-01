<template>
  <div class="hello">
    <h1>{{ msg }}</h1>
    <tr>
    <td>
	    <select v-model="format.format">
		<option v-for="o in formats" :selected="o.format === format.format">{{o.format}}</option>
	    </select>
	    {{ format.description }} {{ format.width }}x{{ format.height }}
    </td>
    </tr>
    <tr v-for="c in controls">
	<td>{{ c.name }}</td>
	<td><input v-model="c.value" type="text"></td>
	<td>[ {{ c.minimum }} , </td>
	<td>{{ c.maximum }} ]</td>
	<td>
		<select v-if="c.menu" v-model="c.value" >
			<option v-for="m in c.menu" >{{ m.label }}</option>
		</select>
		<input v-if="!c.menu && ( (c.minimum==0) && (c.maximum==1) )"
				v-model="c.value" 
				type="checkbox" />	
		<input v-if="!c.menu && !( (c.minimum==0) && (c.maximum==1) )"
				v-model="c.value"
				:min="c.minimum"
				:max="c.maximum"
				type="range" 
				:step="c.step" />						
	</td>
    </tr>    
  </div>
</template>

<script>
import axios from "axios";

var serviceurl = 'http://192.168.11.3:9999'

export default {
  name: 'HelloWorld',
  mounted() {
        axios({ method: "GET", "url": serviceurl+ "/capabilities"}).then(response => {
		this.msg = response.data.card;
        });
        axios({ method: "GET", "url": serviceurl+ "/formats"}).then(response => {
		this.formats = response.data;

		axios({ method: "GET", "url": serviceurl+ "/format"}).then(response => {
			this.format = response.data;
			this.formats.forEach( item => { 
				if (item.format === this.format.format ) {
					this.format.description = item.description
				} 
			} );
		});
        });
        axios({ method: "GET", "url": serviceurl+ "/controls"}).then(response => {
		this.controls = response.data;
	});
    },
  data () {
    return {
      msg: 'loading...',
      formats: [],
      format: "",
      controls: [],
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
