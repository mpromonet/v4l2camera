import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-controls',
  template: `
   <div>
	<table>
		<thead>
			<tr>
				<th>Name</th>
				<th>Current</th>
				<th>Min</th>
				<th>Max</th>
				<th>Value</th>
			</tr>
		</thead>
		<tbody>
			<tr *ngFor="let control of controlList" >
				<td>{{control.name}}</td>
				<td>
					<input #name placeholder="{{control.value}}" type="text" size="8"/>				
				</td>
				<td>{{control.minimum}}</td>
				<td>{{control.maximum}}</td>
				<td>
					<!-- combo for enumerate -->
					<select *ngIf="control.menu" [value]="control.value" style="width: 300px">
							<option *ngFor="let menu of control.menu" [value]="menu.value">{{ menu.label }}</option>
					</select>
					<!-- checkbox for [0,1] -->
					<input *ngIf="!control.menu && ( (control.minimum==0) && (control.maximum==1) )"
							[checked]="control.value" 
							type="checkbox" />							
					<!-- slider for others -->
					<input *ngIf="!control.menu && !( (control.minimum==0) && (control.maximum==1) )"
							[value]="control.value"
							ng-trim="false" 
							min="{{control.minimum}}"
							max="{{control.maximum}}"
							type="range" 
							step="{{control.step}}"
							style="width: 300px; margin: 0px"/>							
				</td>
			</tr>      
		</tbody>
	</table>
</div>
  `,
  styles: []
})

export class ControlsComponent implements OnInit {

	controlList = {};
	constructor(private _httpClient: HttpClient) {
	}
	
	ngOnInit() {
		this._httpClient.get("http://127.0.0.1:8080/controls")
		    .subscribe(data => {
			     this.controlList = data;
		    });
	}	
}
