import {Component, OnInit, ElementRef} from '@angular/core';
import {CommonModule} from '@angular/common';
import {Router} from '@angular/router';
import {DomSanitizer}from '@angular/platform-browser';
import {MatIconRegistry, MatIconModule} from '@angular/material/icon';

export interface Settings {
  id: number;
  favorites: bigint;
}

export interface MenuItem {
  target: string;
  description: string;
  descriptionLineBreak: string;
  class : string;
  index: number;
}
  
@Component({
  selector: 'app-menu',
  templateUrl: './menu.component.html',
  styleUrls: ['./menu.component.sass'],
  imports: [MatIconModule, CommonModule],
  standalone: true,
})
export class MenuComponent implements OnInit{

  public checked : boolean[] =[];
  public favoriteList : MenuItem[] =[];
  favorites: number = 0;
  settings: Settings = {id: 0, favorites: 0n};

  targetedSubmenu: string = "one";
  targetedSubSubmenu: string = "";
  storeTargetedSubmenu: string = "";
  storeTargetedSubSubmenu: string = "";
  targetedSubSubSubmenu: string = "";
  storeTargetedSubSubSubmenu: string = "";

  constructor(
    private matIconRegistry: MatIconRegistry,
    private domSanitizer: DomSanitizer,
    private elementRef: ElementRef,
    public router: Router) {
    this.elementRef.nativeElement.style.setProperty('--menu-width', '200px');              
    this.matIconRegistry.addSvgIcon("custom_check", this.domSanitizer.bypassSecurityTrustResourceUrl("./assets/svg/stop.svg"));
    this.matIconRegistry.addSvgIcon("custom_menu",this.domSanitizer.bypassSecurityTrustResourceUrl("./assets/svg/hamburger.svg"));
    this.matIconRegistry.addSvgIcon("custom_star",this.domSanitizer.bypassSecurityTrustResourceUrl("./assets/svg/star.svg"));
  }

  ngOnInit() {  
    if(this.router.url == '/imgui' || 
      this.router.url == '/open-gl' || 
      this.router.url == '/cube' || 
      this.router.url == '/triangle' || 
      this.router.url == '/shape')
    {
      this.targetedSubmenu = 'two';
      this.storeTargetedSubmenu = 'two';     
    }

    if(this.router.url == '/webgpu/wireframe' || 
      this.router.url == '/webgpu/specularity' ||
      this.router.url == '/webgpu/compute' ||
      this.router.url == '/webgpu/normal-map' ||
      this.router.url == '/webgpu/msdf-font' ||
      this.router.url == '/webgpu/instanced-cube'||
      this.router.url == '/webgpu/ibl' ||
      this.router.url == '/webgpu/shadow-mapping' ||
      this.router.url == '/webgpu/skinned-mesh' ||
      this.router.url == '/webgpu/cpl' ||
      this.router.url == '/webgpu/primitive-picking' ||
      this.router.url == '/webgpu/stencil-mask' ||
      this.router.url == '/webgpu/deferred-rendering' ||
      this.router.url == '/webgpu/volume-rendering' ||
      this.router.url == '/webgpu/occlusion-query' ||  
      this.router.url == '/webgpu/render-bundles' ||
      this.router.url == '/webgpu/nuklear-ui' ||    
      this.router.url == '/webgpu/audio-decode' ||
      this.router.url == '/webgpu/video-decode' ||
      this.router.url == '/webgpu/cubes' ||
      this.router.url == '/webgpu/isometric' ||
      this.router.url == '/webgpu/menu' ||
      this.router.url == '/webgpu/controller' ||
      this.router.url == '/webgpu/hdr-bloom' ||
      this.router.url == '/webgpu/abuffer' ||
      this.router.url == '/webgpu/cpe' ||
      this.router.url == '/webgpu/pristine-grid' ||
      this.router.url == '/webgpu/wireframe-overlay'||
      this.router.url == '/webgpu/volumetric-texture')
    {
      this.targetedSubmenu = 'three';
      this.storeTargetedSubmenu = 'three';     
    }
  }

  ngAfterViewInit(){
    this.refreshItems();       
  }

  refreshItems(){
    this.favoriteList = [];
    this.checked = [];
    let map = new Map<bigint, MenuItem>()
    .set(1n << 0n, {target : '/imgui', description: 'ImGUI', descriptionLineBreak: '', class : 'two-one', index : 0} as MenuItem)
    .set(1n << 1n, {target : '/open-gl', description: 'OpenGL', descriptionLineBreak: '', class : 'two-two', index : 1} as MenuItem)
    .set(1n << 2n, {target : '/cube', description: 'Cube', descriptionLineBreak: '', class : 'two-three', index : 2} as MenuItem)
    .set(1n << 3n, {target : '/triangle', description: 'Triangle', descriptionLineBreak: '', class : 'two-four', index : 3} as MenuItem)
    .set(1n << 4n, {target : '/shape', description: 'Shape', descriptionLineBreak: '', class : 'two-eight', index : 4} as MenuItem)
    .set(1n << 5n, {target : '/webgpu/wireframe', description: 'Wireframe', descriptionLineBreak: '', class : 'three-one', index : 5} as MenuItem)
    .set(1n << 6n, {target : '/webgpu/specularity', description: 'Specularity', descriptionLineBreak: '', class : 'three-two', index : 6} as MenuItem)
    .set(1n << 7n, {target : '/webgpu/compute', description: 'Compute', descriptionLineBreak: '', class : 'three-three', index : 7} as MenuItem)
    .set(1n << 8n, {target : '/webgpu/normal-map', description: 'Normal Mapping', descriptionLineBreak: '', class : 'three-four', index : 8} as MenuItem)
    .set(1n << 9n, {target : '/webgpu/msdf-font', description: 'MSDF Font', descriptionLineBreak: '', class : 'three-five', index : 9} as MenuItem)
    .set(1n << 10n, {target : '/webgpu/instanced-cube', description: 'Instanced Cube', descriptionLineBreak: '', class : 'three-six', index : 10} as MenuItem)
    .set(1n << 11n, {target : '/webgpu/ibl', description: 'Image Based Lighting', descriptionLineBreak: '', class : 'three-seven', index : 11} as MenuItem)
    .set(1n << 12n, {target : '/webgpu/shadow-mapping', description: 'Shadow Mapping', descriptionLineBreak: '', class : 'three-eight', index : 12} as MenuItem)
    .set(1n << 13n, {target : '/webgpu/skinned-mesh', description: 'Skinned Mesh', descriptionLineBreak: '', class : 'three-nine', index : 13} as MenuItem)
    .set(1n << 14n, {target : '/webgpu/cpl', description: 'Compute Particle Logo', descriptionLineBreak: '', class : 'three-ten', index : 14} as MenuItem)
    .set(1n << 15n, {target : '/webgpu/primitive-picking', description: 'Primitive Picking', descriptionLineBreak: '', class : 'three-eleven', index : 15} as MenuItem)
    .set(1n << 16n, {target : '/webgpu/stencil-mask', description: 'Stencil Mask', descriptionLineBreak: '', class : 'three-twelve', index : 16} as MenuItem)
    .set(1n << 17n, {target : '/webgpu/deferred-rendering', description: 'Deferred Rendering', descriptionLineBreak: '', class : 'three-thirteen', index : 17} as MenuItem)
    .set(1n << 18n, {target : '/webgpu/volume-rendering', description: 'Volume Rendering', descriptionLineBreak: '', class : 'three-fourteen', index : 18} as MenuItem)
    .set(1n << 19n, {target : '/webgpu/occlusion-query', description: 'Occlusion Query', descriptionLineBreak: '', class : 'three-fifteen', index : 19} as MenuItem) 
    .set(1n << 20n, {target : '/webgpu/render-bundles', description: 'Render Bundles', descriptionLineBreak: '', class : 'three-sixteen', index : 20} as MenuItem)
    .set(1n << 21n, {target : '/webgpu/nuklear-ui', description: 'Nuklear UI', descriptionLineBreak: '', class : 'three-seventeen', index : 21} as MenuItem)
    .set(1n << 22n, {target : '/webgpu/audio-decode', description: 'Audio Decode', descriptionLineBreak: '', class : 'three-eightteen', index : 22} as MenuItem)
    .set(1n << 23n, {target : '/webgpu/video-decode', description: 'Video Decode', descriptionLineBreak: '', class : 'three-ninenteen', index : 23} as MenuItem)
    .set(1n << 24n, {target : '/webgpu/cubes', description: 'Cubes', descriptionLineBreak: '', class : 'three-twenty', index : 24} as MenuItem)
    .set(1n << 25n, {target : '/webgpu/isometric', description: 'Isometric', descriptionLineBreak: '', class : 'three-twentyone', index : 25} as MenuItem)
    .set(1n << 26n, {target : '/webgpu/menu', description: 'Menu', descriptionLineBreak: '', class : 'three-twentytwo', index : 26} as MenuItem)
    .set(1n << 27n, {target : '/webgpu/controller', description: 'Controller', descriptionLineBreak: '', class : 'three-twentythree', index : 27} as MenuItem)
    .set(1n << 28n, {target : '/webgpu/hdr-bloom', description: 'HDR Bloom', descriptionLineBreak: '', class : 'three-twentyfour', index : 28} as MenuItem)
    .set(1n << 29n, {target : '/webgpu/abuffer', description: 'A Buffer', descriptionLineBreak: '', class : 'three-twentyfive', index : 29} as MenuItem)
    .set(1n << 30n, {target : '/webgpu/cpe', description: 'Compute Particle Easing', descriptionLineBreak: '', class : 'three-twentysix', index : 30} as MenuItem)
    .set(1n << 31n, {target : '/webgpu/pristine-grid', description: 'Pristine Grid', descriptionLineBreak: '', class : 'three-twentyseven', index : 31} as MenuItem)
    .set(1n << 32n, {target : '/webgpu/wireframe-overlay', description: 'Wireframe Overlay', descriptionLineBreak: '', class : 'three-twentyeight', index : 32} as MenuItem)
    .set(1n << 33n, {target : '/webgpu/volumetric-texture', description: 'Volumetric Texture', descriptionLineBreak: '', class : 'three-thirty', index : 33} as MenuItem);
    
    for(let key of map.keys()) {
      if((this.settings.favorites | key) == this.settings.favorites){
        this.checked.push(true);       
        this.favoriteList.push(map.get(key) as MenuItem);
      }else{
        this.checked.push(false);
      }  
    }
  }

  onClick(event: any) {  
    this.targetedSubmenu = event.target.parentElement.id != "" ? event.target.parentElement.id  : event.target.parentElement.parentElement.id != "" ? event.target.parentElement.parentElement.id : event.target.parentElement.parentElement.parentElement.id;
    
    if(this.targetedSubmenu.includes('-') && !this.targetedSubmenu.includes('favorite-')){
      return;
    }

    if(this.targetedSubmenu === this.storeTargetedSubmenu && this.targetedSubmenu !== ''){
      this.targetedSubmenu = '';
      this.storeTargetedSubmenu = '';
      return;
    }

    this.targetedSubSubmenu = '';
    this.storeTargetedSubSubmenu = '';

    this.targetedSubSubSubmenu = '';
    this.storeTargetedSubSubSubmenu = '';

    if(this.targetedSubmenu.includes('favorite-')){
      let id = this.targetedSubmenu;
      this.storeTargetedSubSubSubmenu =  id.replace('favorite-', '') 

      let array = this.storeTargetedSubSubSubmenu.split('-');

      if(array.length == 4){
        this.storeTargetedSubSubSubmenu = array[0] + '-' + array[1] + '-' + array[2];
        this.storeTargetedSubSubmenu = array[0] + '-' + array[1];
      }
      
      if(array.length == 3){
        this.storeTargetedSubSubmenu = array[0] + '-' + array[1];
      }

      this.targetedSubmenu = array[0];
    }

    if(this.targetedSubmenu.includes('-')){
      this.targetedSubmenu = this.targetedSubmenu.split('-')[0];  
    }

    if(this.targetedSubmenu == ''){
      this.targetedSubmenu = this.storeTargetedSubmenu;
    }else{
      this.storeTargetedSubmenu = this.targetedSubmenu;
    }
    return;
  }

  onClickSub(event: any) {  
    this.targetedSubSubmenu = event.target.parentElement.id != "" ? event.target.parentElement.id  : event.target.parentElement.parentElement.id != "" ? event.target.parentElement.parentElement.id : event.target.parentElement.parentElement.parentElement.id;

    if(this.targetedSubSubmenu == this.storeTargetedSubSubmenu && this.targetedSubSubmenu != ''){
      this.targetedSubSubmenu = '';
      this.storeTargetedSubSubmenu = '';
    
      this.targetedSubSubSubmenu = '';
      this.storeTargetedSubSubSubmenu = '';
      return;
    }

    if(this.targetedSubSubmenu.includes('-')){
      this.targetedSubSubmenu = this.targetedSubSubmenu.split('-')[0] + '-' + this.targetedSubSubmenu.split('-')[1];  
    }

    if(this.targetedSubSubmenu == ''){
      this.targetedSubSubmenu = this.storeTargetedSubSubmenu;
    }else{
      this.storeTargetedSubSubmenu = this.targetedSubSubmenu;
    }
    return;
  }

  onClickSubSub(event: any) {  
    this.targetedSubSubSubmenu = event.target.parentElement.id != "" ? event.target.parentElement.id  : event.target.parentElement.parentElement.id != "" ? event.target.parentElement.parentElement.id : event.target.parentElement.parentElement.parentElement.id;

    if(this.targetedSubSubSubmenu == this.storeTargetedSubSubSubmenu && this.targetedSubSubSubmenu != ''){
      this.targetedSubSubSubmenu = '';
      this.storeTargetedSubSubSubmenu = '';
      return;
    }

    if(this.targetedSubSubSubmenu.includes('-')){
      this.targetedSubSubSubmenu = this.targetedSubSubSubmenu.split('-')[0] + '-' + this.targetedSubSubSubmenu.split('-')[1]+ '-' + this.targetedSubSubSubmenu.split('-')[2];  
    }

    if(this.targetedSubSubSubmenu == ''){
      this.targetedSubSubSubmenu = this.storeTargetedSubSubSubmenu;
    }else{
      this.storeTargetedSubSubSubmenu = this.targetedSubSubSubmenu;
    }
    return;
  }

  updateFavorites(event: any, _number: number, checked: boolean, menuItem?: MenuItem){
    let settings: Settings = this.settings;
    const index = BigInt(menuItem ? menuItem.index : _number);
    const bitValue = 1n << index;
    let currentFavorites = BigInt(settings.favorites);

    if (checked) {       
      currentFavorites = currentFavorites ^ bitValue;
    } else {
      currentFavorites = currentFavorites | bitValue;        
    }

    settings.favorites = currentFavorites;
    this.postSettings(settings);     
  }

  postSettings(settings: Settings) {      
    this.refreshItems();
  }

  navigate(route: string){
    this.router.navigateByUrl(route);
  }

  logout(){
    window.location.href = 'https://github.com/pohldaniel/angular-emscripten';
  }
}
  