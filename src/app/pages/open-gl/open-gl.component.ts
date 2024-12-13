import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";

@Component({
  selector: 'app-open-gl',
  templateUrl: './open-gl.component.html',
  styleUrls: ['./open-gl.component.sass'],
  standalone: true,
})
export class OpenGLComponent extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("OpenGLModule",  'assets/wasm/02OpenGL/open-gl.js', 'assets/wasm/02OpenGL/open-gl.wasm', 'assets/wasm/02OpenGL/open-gl.data');
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