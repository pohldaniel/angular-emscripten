import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone} from '@angular/core';
import {NgStyle} from '@angular/common';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";
import {ResizedDirective} from 'src/app/directives/resized.directive';
import {ResizedEvent} from 'src/app/directives/resized.directive';
import {SpinnerComponent} from 'src/app/shared/spinner/spinner.component';

@Component({
  selector: 'app-compute',
  templateUrl: './compute.component.html',
  styleUrls: ['./compute.component.sass'],
  imports: [ResizedDirective, SpinnerComponent, NgStyle],
  standalone: true,
})
export class ComputeComponent extends EmscriptenWasmComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('canvas') canvas!: ElementRef;
  @ViewChild('span') span!: ElementRef;
  error!: string;
  moduleLoaded: boolean = false;

  constructor(private ngZone: NgZone) {
    super("ComputeModule",  'assets/wasm/07Compute/webgpu.js', 'assets/wasm/07Compute/webgpu.wasm', 'assets/wasm/07Compute/webgpu.data');
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
    this.loadModule();
  }
  
  loadModule(){
    setTimeout(() => {  
      if(this.module && this.module.ccall!('IsInitialized', 'Boolean', [], [])){ 
        this.canvas.nativeElement.width  = this.span.nativeElement.clientWidth - 5;
        this.canvas.nativeElement.height = this.span.nativeElement.clientHeight - 5;
        this.module.ccall!('Resize', 'void', ['number', 'number'], [this.canvas.nativeElement.width, this.canvas.nativeElement.height]);
        this.moduleLoaded = true; 
      }else{
        this.loadModule();
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