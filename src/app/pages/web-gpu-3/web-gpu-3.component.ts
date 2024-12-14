import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";

@Component({
  selector: 'app-web-gpu-3',
  templateUrl: './web-gpu-3.component.html',
  styleUrls: ['./web-gpu-3.component.sass'],
  standalone: true,
})
export class WebGPU3Component extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("WebGPU3Module",  'assets/wasm/07WebGPU/web-gpu.js', 'assets/wasm/07WebGPU/web-gpu.wasm', 'assets/wasm/07WebGPU/web-gpu.data');
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