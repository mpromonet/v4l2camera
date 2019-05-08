import { Component, OnInit } from '@angular/core';
import { WebsocketService } from "../websocket.service";
import * as Rx from 'rxjs';

@Component({
  selector: 'app-video',
  template: `
    <img [src]="image" >
  `,
  styles: []
})
export class VideoComponent implements OnInit {
   image = "";
	
  constructor(wsService: WebsocketService) { 
	let ws = wsService.connect("ws://127.0.0.1:8080/ws");
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

  ngOnInit() {
  }

}
