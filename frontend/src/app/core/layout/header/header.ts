import { Component, inject } from '@angular/core';
import { WasmService } from '../../services/wasm.service';

@Component({
  imports: [],
  selector: 'app-header',
  styleUrl: './header.scss',
  templateUrl: './header.html',
})
export class Header {
  readonly wasmService = inject(WasmService);
}
