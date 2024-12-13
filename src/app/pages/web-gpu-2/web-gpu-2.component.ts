import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";

@Component({
  selector: 'app-web-gpu-2',
  templateUrl: './web-gpu-2.component.html',
  styleUrls: ['./web-gpu-2.component.sass'],
  standalone: true,
})
export class WebGPU2Component extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("WebGPU2Module",  'assets/wasm/06WebGPU/web-gpu.js', 'assets/wasm/06WebGPU/web-gpu.wasm', 'assets/wasm/06WebGPU/web-gpu.data');
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