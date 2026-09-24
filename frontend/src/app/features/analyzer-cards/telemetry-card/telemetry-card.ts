import { Component, input } from '@angular/core';
import { Footer } from '../../../core/layout/footer/footer';
import { ExpressionMetrics } from '../../../core/models/expression.model';

@Component({
  imports: [Footer],
  selector: 'app-telemetry-card',
  styleUrl: './telemetry-card.scss',
  templateUrl: './telemetry-card.html',
})
export class TelemetryCard {
  readonly metrics = input<ExpressionMetrics | null>(null);
}
