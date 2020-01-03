import { Component, OnInit, Inject } from '@angular/core';
import { WebsocketService } from "../websocket.service";
import { DOCUMENT } from '@angular/common';

@Component({
  selector: 'app-video',
  template: `
    <img [src]="image" >
  `,
  styles: []
})
export class VideoComponent implements OnInit {
   image = "";
	
  constructor(@Inject(DOCUMENT) private _document: any,
  				private wsService: WebsocketService) { 
  }

  ngOnInit() {
	const wsurl = this._document.location.href.replace("http","ws") + "/ws";  
	let ws = this.wsService.connect(wsurl);
	ws.subscribe((response: MessageEvent) => {
		let data = response.data;
		if (data instanceof Blob) {
			new Response(data).arrayBuffer().then( (binary) => {
				var bytes = new Uint8Array(binary);	
				var binaryStr = "";
				for(let i=0;i<bytes.length;i++) binaryStr += String.fromCharCode(bytes[i]);
				this.image = 'data:image/jpeg;base64,' + btoa(binaryStr);
			})
		}
	});
  }

}
