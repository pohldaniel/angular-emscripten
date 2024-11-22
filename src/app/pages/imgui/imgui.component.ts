import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {EmscriptenWasmComponent} from 'src/app/emscripten-wasm.component';

@Component({
  selector: 'app-imgui-cpp',
  templateUrl: './imgui.component.html',
  styleUrls: ['./imgui.component.sass'],
  standalone: true,
})
export class ImguiComponent extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  error!: string;
  constructor(private ngZone: NgZone) {
    super("ImguiWebModule",  'assets/wasm/imgui/ImguiWeb.js', 'assets/wasm/imgui/ImguiWeb.wasm');
    this.moduleDecorator = (mod) => {     
      mod.canvas = <HTMLCanvasElement>this.canvas.nativeElement;
      mod.printErr = (what: string) => {
        if (!what.startsWith("WARNING")) {
          this.ngZone.run(() => (this.error = what));
        }
      };
    };
  }
}