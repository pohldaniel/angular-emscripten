import {Routes} from '@angular/router';
import {authGuard} from './guard/auth.guard';
import {ImguiComponent} from './pages/imgui/imgui.component';
import {OpenGLComponent} from './pages/open-gl/open-gl.component';
import {CubeComponent} from './pages/cube/cube.component';
import {TriangleComponent} from './pages/triangle/triangle.component';
import {WireframeComponent} from './pages/webgpu/wireframe/wireframe.component';
import {SpecularityComponent} from './pages/webgpu/specularity/specularity.component';
import {ComputeComponent} from './pages/webgpu/compute/compute.component';
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
  {
    path: '**',
    redirectTo: 'webgpu/wireframe'
  } 
];
