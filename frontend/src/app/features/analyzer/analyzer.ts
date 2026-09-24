import { Component, signal } from '@angular/core';
import { ExpressionMetrics } from '../../core/models/expression.model';

import { VideoCard } from '../analyzer-cards/video-card/video-card';
import { TelemetryCard } from '../analyzer-cards/telemetry-card/telemetry-card';

@Component({
  imports: [VideoCard, TelemetryCard],
  selector: 'app-analyzer',
  styleUrl: './analyzer.scss',
  templateUrl: './analyzer.html',
})
export class Analyzer {
  readonly metrics = signal<ExpressionMetrics | null>(null);
}
