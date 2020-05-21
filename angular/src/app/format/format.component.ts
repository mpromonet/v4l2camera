import { Component, OnInit, Inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DOCUMENT } from '@angular/common';

@Component({
  selector: 'app-format',
  template: `
    	<table>
		<tbody>
			<tr>
				<td>format</td>
				<td>				
					<select [value]="selectedFormat.format"	(change)="update($event.target.value, selectedFormat.width, selectedFormat.height, selectedFormat.fps)" >		
						<option *ngFor="let format of formatList" [value]="format.format">{{ format.format }}</option>
					</select>
				</td>
				<td>
					{{selectedFormat.description}}
				</td>
			</tr>
			<tr>				
				<td>width</td>
				<td>
					<input  [value]="selectedFormat.width"  
					(change)="update(selectedFormat.format, $event.target.value, selectedFormat.height, selectedFormat.fps)"
					type="text" size="5"/>
				</td>
			</tr>
			<tr>				
				<td>height</td>
				<td>
					<input  [value]="selectedFormat.height"  
					(change)="update(selectedFormat.format, selectedFormat.width, $event.target.value, selectedFormat.fps)"
					type="text" size="5"/>
				</td>
			</tr>
			<tr>
				<td>fps</td>
				<td>
					<input  [value]="selectedFormat.fps" 
							(change)="update(selectedFormat.format, selectedFormat.width, selectedFormat.height, $event.target.value)" 
							type="text" size="5"/>
				</td>
			</tr>
		</tbody>
	</table>
  `,
  styles: []
})
export class FormatComponent implements OnInit {

 	formatList = {};
	selectedFormat : {
		format: string,
		description: string,
		width: number,
		height: number,
		fps: number
	} = {
		format: "???",
		description: "",
		width: 0,
		height: 0,
		fps: 0
	};
	
	constructor(@Inject(DOCUMENT) private _document: any, 
				private _httpClient: HttpClient) {
	}

	ngOnInit() {
		this._httpClient.get(this._document.location.href + "/api/formats")
		    .subscribe(data => {
				this.formatList = data;
				this._httpClient.get(this._document.location.href + "/api/format").subscribe(data => {
					     (this.selectedFormat as any) = data;
				});
		    });
		    
	}

	update(format: string, widthstr: any, heightstr: any, fpsstr: any) {
		const width = parseInt(widthstr)
		const height = parseInt(heightstr)
		const fps = parseInt(fpsstr)		
		console.log(format, width, height, fps)
		for (const prop in this.selectedFormat) {
			this.selectedFormat[prop] = "???"
		}
		this._httpClient.post(this._document.location.href + "/api/format", {format, width, height, fps}).subscribe((data: any) => {
			this.selectedFormat = data;
  		 });		
	}
}
