import {Component, OnInit, AfterViewInit,ViewChild,ElementRef, OnDestroy, NgZone, Renderer2} from '@angular/core';
import {NgStyle} from '@angular/common';
import {EmscriptenWasmComponent} from "src/app/emscripten-wasm.component";
import {ResizedDirective} from 'src/app/directives/resized.directive';
import {ResizedEvent} from 'src/app/directives/resized.directive';
import {SpinnerComponent} from 'src/app/shared/spinner/spinner.component';

@Component({
  selector: 'app-specularity',
  templateUrl: './specularity.component.html',
  styleUrls: ['./specularity.component.sass'],
  imports: [ResizedDirective, SpinnerComponent, NgStyle],
  standalone: true,
})
export class SpecularityComponent extends EmscriptenWasmComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('canvas') canvas!: ElementRef;
  @ViewChild('span') span!: ElementRef;
  error!: string;
  moduleLoaded: boolean = false;
  useNativeSize: boolean = true;

  constructor(private ngZone: NgZone, private renderer: Renderer2) {
    super("SpecularityModule",  'assets/wasm/06Specularity/webgpu.js', 'assets/wasm/06Specularity/webgpu.wasm', 'assets/wasm/06Specularity/webgpu.data');
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
    let width = getComputedStyle(this.span.nativeElement).getPropertyValue('width');
    let height = getComputedStyle(this.span.nativeElement).getPropertyValue('height');
    let widthN = parseInt(width.substring(0, width.length - 2));
    let heightN = parseInt(height.substring(0, height.length - 2));
    this.loadModule(widthN , heightN);
  }
    
  loadModule(width: number, height: number){
    setTimeout(() => {  
      if(this.module && this.module.ccall!('IsInitialized', 'boolean', [], [])){ 
        this.canvas.nativeElement.width  = this.useNativeSize ? this.module.ccall!('GetWidth', 'number', [], []) as number : width - 5;
        this.canvas.nativeElement.height = this.useNativeSize ? this.module.ccall!('GetHeight', 'number', [], []) as number : height - 5;
        
        if(this.useNativeSize){
          this.renderer.setStyle(this.span.nativeElement, 'width', this.canvas.nativeElement.width + 5 + 'px');
          this.renderer.setStyle(this.span.nativeElement, 'height', this.canvas.nativeElement.height + 5 + 'px');
        }

        this.module.ccall!('Resize', 'void', ['number', 'number'], [this.canvas.nativeElement.width, this.canvas.nativeElement.height]); 
        this.moduleLoaded = true;
      }else{
          this.loadModule(width, height);
      }
    }, 100);
  }
    
  onResized(event: ResizedEvent) {
    if(this.moduleLoaded){   
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