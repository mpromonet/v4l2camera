import { Component, OnInit, Inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DOCUMENT } from '@angular/common';

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
			<tr *ngFor="let control of controlList; let index = index" >
				<td>{{control.name}}</td>
				<td>
					<input #name placeholder="{{control.value}}" type="text" size="8"/>				
				</td>
				<td>{{control.minimum}}</td>
				<td>
					<!-- combo for enumerate -->
					<select *ngIf="control.menu" 
							[value]="control.value" (change)="changed(control.id,$event.target.value,index)" 
							style="width: 300px">
								<option *ngFor="let menu of control.menu" [value]="menu.value">{{ menu.label }}</option>
					</select>
					<!-- checkbox for [0,1] -->
					<input *ngIf="!control.menu && ( (control.minimum==0) && (control.maximum==1) )"
							[checked]="control.value" (change)="changed(control.id,$event.target.checked?1:0,index)"
							type="checkbox" />							
					<!-- slider for others -->
					<input *ngIf="!control.menu && !( (control.minimum==0) && (control.maximum==1) )"
							[value]="control.value" (change)="changed(control.id,$event.target.value,index)"
							ng-trim="false" 
							min="{{control.minimum}}"
							max="{{control.maximum}}"
							type="range" 
							step="{{control.step}}"
							style="width: 300px; margin: 0px"/>							
				</td>
				<td>{{control.maximum}}</td>
			</tr>      
		</tbody>
	</table>
</div>
  `,
  styles: []
})

export class ControlsComponent implements OnInit {

	controlList = {};
	constructor(@Inject(DOCUMENT) private _document: any, 
				private _httpClient: HttpClient) {
	}
	
	ngOnInit() {
		this._httpClient.get(this._document.location.href + "/api/controls")
		    .subscribe(data => {
			     this.controlList = data;
		    });
	}	

	changed(id: string, value: any, idx: number) {
		value = parseInt(value)
		this._httpClient.post(this._document.location.href + "/api/control", {id, value}).subscribe( (data: any) => {
			    this.controlList[idx].value = data.value;		
		})
	}
}
