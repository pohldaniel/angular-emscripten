import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";

@Component({
  selector: 'app-web-gpu',
  templateUrl: './web-gpu.component.html',
  styleUrls: ['./web-gpu.component.sass'],
  standalone: true,
})
export class WebGPUComponent extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("WebGPUModule",  'assets/wasm/05WebGPU/web-gpu.js', 'assets/wasm/05WebGPU/web-gpu.wasm', 'assets/wasm/05WebGPU/web-gpu.data');
    this.moduleDecorator = (mod) => {     
      mod.canvas = <HTMLCanvasElement >this.canvas.nativeElement;
      mod.printErr = (what: string) => {
        if (!what.startsWith("WARNING")) {
          this.ngZone.run(() => (this.error = what));
        }
      };
    };
  }
}