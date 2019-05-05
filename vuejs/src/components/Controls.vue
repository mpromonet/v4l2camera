<template>
<div>
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

var serviceurl = ''

export default {
  mounted() {
        axios({ method: "GET", "url": serviceurl+ "/controls"}).then(response => {
		this.controls = response.data;
	});
    },
  data () {
    return {
      controls: [],
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
