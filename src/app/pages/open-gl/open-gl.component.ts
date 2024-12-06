import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";

@Component({
  selector: 'app-open-gl-cpp',
  templateUrl: './open-gl.component.html',
  styleUrls: ['./open-gl.component.sass'],
  standalone: true,
})
export class OpenComponentGL extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("OpenModule",  'assets/wasm/openGL/open-gl.js', 'assets/wasm/openGL/open-gl.wasm', 'assets/wasm/openGL/open-gl.data');
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