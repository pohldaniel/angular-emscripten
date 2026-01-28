import {Component, OnInit, AfterViewInit, OnDestroy} from '@angular/core';
import {EmscriptenWasmComponent} from 'src/app/emscripten-wasm.component';
declare var Module: any;

@Component({
  selector: 'app-triangle',
  templateUrl: './triangle.component.html',
  styleUrls: ['./triangle.component.scss'],
  standalone: true,
})
export class TriangleComponent extends EmscriptenWasmComponent implements OnInit, AfterViewInit, OnDestroy {
 
  constructor() {
    super("TriangleModule",  'assets/wasm/04Triangle/triangle.js', 'assets/wasm/04Triangle/triangle.wasm');
  }

  override ngOnInit() {
    
  }

  override ngAfterViewInit() {
    super.ngAfterViewInit();
  }
  
  override ngOnDestroy() {  
  
  }
}