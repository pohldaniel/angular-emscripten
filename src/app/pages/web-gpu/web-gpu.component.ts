import {Component, AfterViewInit, OnInit, OnDestroy, ViewChild,ElementRef, NgZone, Inject} from '@angular/core';
import {EmscriptenWasmComponent} from 'src/app/emscripten-wasm.component';
import {ResizedDirective} from 'src/app/directives/resized.directive';
import {ResizedEvent} from 'src/app/directives/resized.directive';
import {DOCUMENT} from '@angular/common';
@Component({
  selector: 'app-web-gpu',
  templateUrl: './web-gpu.component.html',
  styleUrls: ['./web-gpu.component.sass'],
  imports: [ResizedDirective],
  standalone: true,
})
export class WebGPUComponent extends EmscriptenWasmComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('canvas') canvas!: ElementRef;
  @ViewChild('span') span!: ElementRef;
  error!: string;

  constructor(private ngZone: NgZone, @Inject(DOCUMENT) private document: Document) { 
    super('WebGPUModule',  'assets/wasm/05WebGPU/web-gpu.js', 'assets/wasm/05WebGPU/web-gpu.wasm', 'assets/wasm/05WebGPU/web-gpu.data');
  }
  
  override ngOnInit(): void {
    this.moduleDecorator = (mod) => {     
      mod.canvas = <HTMLCanvasElement >this.canvas.nativeElement;
      mod.printErr = (what: string) => {
        if (!what.startsWith('WARNING')) {
          this.ngZone.run(() => (this.error = what));
        }
      };
    };
  }

  override ngAfterViewInit() {
    super.ngAfterViewInit();
    this.moduleLoaded();
  }

  moduleLoaded(){
    setTimeout(() => {  
      if(this.module && this.module.ccall!('IsInitialized', 'Boolean', [], [])){ 
        this.canvas.nativeElement.width  = this.span.nativeElement.clientWidth - 5;
        this.canvas.nativeElement.height = this.span.nativeElement.clientHeight - 5;
        this.module.ccall!('Resize', 'void', ['number', 'number'], [this.canvas.nativeElement.width, this.canvas.nativeElement.height]); 
      }else{
        this.moduleLoaded();
      }
    }, 100);
  }

  onResized(event: ResizedEvent) {
    if(this.module){   
      this.canvas.nativeElement.width  = event.newRect.width - 5;
      this.canvas.nativeElement.height = event.newRect.height - 5;
      this.module.ccall!('Resize', 'void', ['number', 'number'], [this.canvas.nativeElement.width, this.canvas.nativeElement.height]); 
    }
  }

  override ngOnDestroy() {
    if(this.module){
      this.module.ccall!('Cleanup', 'void', [], [])
    }
  }
}