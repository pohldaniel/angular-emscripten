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
import {ImageBasedLightingComponent} from './pages/webgpu/image-based-lighting/image-based-lighting.component';
import {ShadowMappingComponent} from './pages/webgpu/shadow-mapping/shadow-mapping.component';
import {SkinnedMeshComponent} from './pages/webgpu/skinned-mesh/skinned-mesh.component';
import {ComputeParticleLogoComponent} from './pages/webgpu/compute-particle-logo/compute-particle-logo.component';
import {PrimitivePickingComponent} from './pages/webgpu/primitive-picking/primitive-picking.component';
import { StencilMaskComponent } from './pages/webgpu/stencil-mask/stencil-mask.component';

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
  { path: 'webgpu/ibl', 
    component: ImageBasedLightingComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/shadow-mapping', 
    component: ShadowMappingComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/skinned-mesh', 
    component: SkinnedMeshComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/cpl', 
    component: ComputeParticleLogoComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/primitive-picking', 
    component: PrimitivePickingComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'webgpu/stencil-mask', 
    component: StencilMaskComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  {
    path: '**',
    redirectTo: 'webgpu/stencil-mask'
  } 
];
