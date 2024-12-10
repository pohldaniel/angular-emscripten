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
    super("WebGPUModule",  'assets/wasm/webGPU/web-gpu.js', 'assets/wasm/webGPU/web-gpu.wasm', 'assets/wasm/webGPU/web-gpu.data');
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