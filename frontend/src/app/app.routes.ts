import { Routes } from '@angular/router';
import { Shell } from './core/layout/shell/shell';

export const routes: Routes = [
  // Shell acts as a wrapper for all main pages
  {
    path: '',
    component: Shell,
    children: [
      {
        path: '',
        loadComponent: () => import('./features/analyzer/analyzer').then((m) => m.Analyzer),
      },
    ],
  },
  {
    path: '**',
    redirectTo: '',
  },
];
