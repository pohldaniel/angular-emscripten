import {Routes} from '@angular/router';
import {authGuard} from './guard/auth.guard';
import {ImguiComponent} from './pages/imgui/imgui.component';
import {OpenComponent} from './pages/open/open.component';

export const routes: Routes = [
  { path: 'imgui', 
    component: ImguiComponent,
    canActivate: [authGuard],
    data : {showSidebar: true},
  },  
  { path: 'open', 
    component: OpenComponent,
    canActivate: [authGuard],
    data : {showSidebar: true}
  },
  {
    path: '',
    component: ImguiComponent
  },
  {
    path: '**',
    redirectTo: ''
  } 
];
