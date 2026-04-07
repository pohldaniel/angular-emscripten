import {Routes} from '@angular/router';
import {authGuard} from './guard/auth.guard';
import {ImguiComponent} from './pages/imgui/imgui.component';
import {OpenGLComponent} from './pages/open-gl/open-gl.component';
import {CubeComponent} from './pages/cube/cube.component';
import {TriangleComponent} from './pages/triangle/triangle.component';
import {ShapeComponent} from './pages/shape/shape.component';
import {WireframeComponent} from './pages/webgpu/wireframe/wireframe.component';
import {SpecularityComponent} from './pages/webgpu/specularity/specularity.component';
import {ComputeComponent} from './pages/webgpu/compute/compute.component';
import {NormalMapComponent} from './pages/webgpu/normal-map/normal-map.component';
import {MSDFFontComponent} from './pages/webgpu/msdf-font/msdf-font.component';
import {InstancedCubeComponent} from './pages/webgpu/instanced-cube/instanced-cube.component';

export const routes: Routes = [
  { path: 'imgui', 
    component: ImguiComponent,
    canActivate: [authGuard],
    data : {showSidebar: true},
  },  
  { path: 'open-gl', 
    component: OpenGLComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'cube', 
    component: CubeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'triangle', 
    component: TriangleComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'shape', 
    component: ShapeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/wireframe', 
    component: WireframeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  }, 
  { path: 'webgpu/specularity', 
    component: SpecularityComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/compute', 
    component: ComputeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/normal-map', 
    component: NormalMapComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/msdf-font', 
    component: MSDFFontComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/instanced-cube', 
    component: InstancedCubeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  {
    path: '**',
    redirectTo: 'webgpu/instanced-cube'
  } 
];
