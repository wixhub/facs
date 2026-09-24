import { Routes } from '@angular/router';

export const routes: Routes = [
  {
    path: '',
    loadComponent: () => import('./features/analyzer/analyzer').then((m) => m.Analyzer),
  },
  {
    path: '**',
    redirectTo: '',
  },
];
