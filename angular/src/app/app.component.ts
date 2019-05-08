import { Component } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-root',
  template: `
   <app-controls></app-controls>
   <app-format></app-format>	
   <app-video></app-video>	
  `,
  styles: []
})
export class AppComponent {
}
