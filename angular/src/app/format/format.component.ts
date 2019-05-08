import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-format',
  template: `
    	<table>
		<tbody>
			<tr>
				<td>format</td>
				<td>				
					<select [value]="selectedFormat.format"	>		
						<option *ngFor="let format of formatList" [value]="format.format">{{ format.format }}</option>
					</select>
				</td>
				<td>
					{{selectedFormat.description}}
				</td>
			</tr>
			<tr *ngFor="let item of ['width', 'height']">				
				<td>{{item}}</td>
				<td>
					<input  [value]="selectedFormat[item]"  type="text" size="5"/>
				</td>
			</tr>
			<tr>
				<td>fps</td>
				<td>
					<input  [value]="selectedFormat.fps"  type="text" size="5"/>
				</td>
			</tr>
		</tbody>
	</table>
  `,
  styles: []
})
export class FormatComponent implements OnInit {

 	formatList = {};
	selectedFormat = {};
	
	constructor(private _httpClient: HttpClient) {
	}

	ngOnInit() {
		this._httpClient.get("http://127.0.0.1:8080/formats")
		    .subscribe(data => {
				this.formatList = data;
				this._httpClient.get("http://127.0.0.1:8080/format").subscribe(data => {
					     this.selectedFormat = data;
				    });
		    });
		    
	}
}
