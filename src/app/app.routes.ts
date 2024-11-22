import {Routes} from '@angular/router';
import {authGuard} from './guard/auth.guard';
import {ImguiComponent} from './pages/imgui/imgui.component';
import {OpenComponentGL} from './pages/open-gl/open-gl.component';
import {CubeComponent} from './pages/cube/cube.component';

export const routes: Routes = [
  { path: 'imgui', 
    component: ImguiComponent,
    canActivate: [authGuard],
    data : {showSidebar: true},
  },  
  { path: 'open-gl', 
    component: OpenComponentGL,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  { path: 'cube', 
    component: CubeComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  {
    path: '**',
    redirectTo: 'imgui'
  } 
];
