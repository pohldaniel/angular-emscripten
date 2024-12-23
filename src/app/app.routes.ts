import {Routes} from '@angular/router';
import {authGuard} from './guard/auth.guard';
import {ImguiComponent} from './pages/imgui/imgui.component';
import {OpenGLComponent} from './pages/open-gl/open-gl.component';
import {CubeComponent} from './pages/cube/cube.component';
import {TriangleComponent} from './pages/triangle/triangle.component';
import {WebGPUComponent} from './pages/web-gpu/web-gpu.component';
import {WebGPU2Component} from './pages/web-gpu-2/web-gpu-2.component';
import {WebGPU3Component} from './pages/web-gpu-3/web-gpu-3.component';
import {ShapeComponent} from './pages/shape/shape.component';

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
  { path: 'web-gpu', 
    component: WebGPUComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  }, 
  { path: 'web-gpu-2', 
    component: WebGPU2Component,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'web-gpu-3', 
    component: WebGPU3Component,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'shape', 
    component: ShapeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  {
    path: '**',
    redirectTo: 'shape'
  } 
];
