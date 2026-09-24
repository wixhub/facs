import { TestBed } from '@angular/core/testing';
import { Component, input, output } from '@angular/core';
import { Analyzer } from './analyzer';
import { VideoCard } from '../analyzer-cards/video-card/video-card';
import { TelemetryCard } from '../analyzer-cards/telemetry-card/telemetry-card';
import { ExpressionMetrics } from '../../core/models/expression.model';

// Dummy stub component for VideoCard
@Component({
  selector: 'app-video-card',
  template: '',
})
class DummyVideoCard {
  readonly metricsChange = output<ExpressionMetrics | null>();
}

// Dummy stub component for TelemetryCard
@Component({
  selector: 'app-telemetry-card',
  template: '',
})
class DummyTelemetryCard {
  readonly metrics = input<ExpressionMetrics | null>(null);
}

describe('Analyzer Component', () => {
  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [Analyzer],
    })
      .overrideComponent(Analyzer, {
        remove: { imports: [VideoCard, TelemetryCard] },
        add: { imports: [DummyVideoCard, DummyTelemetryCard] },
      })
      .compileComponents();
  });

  it('should create the analyzer component successfully', () => {
    const fixture = TestBed.createComponent(Analyzer);
    const component = fixture.componentInstance;

    expect(component).toBeTruthy();
  });

  it('should initialize metrics signal with null value by default', () => {
    const fixture = TestBed.createComponent(Analyzer);
    const component = fixture.componentInstance;

    expect(component.metrics()).toBeNull();
  });

  it('should update metrics signal when new expression data is provided', () => {
    const fixture = TestBed.createComponent(Analyzer);
    const component = fixture.componentInstance;

    const mockMetrics: ExpressionMetrics = {
      dominantEmotion: 'Joy',
      valenceScore: 0.9,
      arousalScore: 0.8,
      actionUnits: {
        au1_innerBrowRaiser: 0,
        au2_outerBrowRaiser: 0,
        au4_browLowerer: 0,
        au5_upperLidRaiser: 0,
        au6_cheekRaiser: 1,
        au9_noseWrinkler: 0,
        au12_lipCornerPuller: 1,
        au15_lipCornerDepressor: 0,
        au17_chinRaiser: 0,
        au25_lipsPart: 0.5,
      },
      ratios: {
        eyeApertureRatio: 0.5,
        mouthWidthRatio: 0.7,
        browFurrowDistance: 0.6,
      },
    };

    // Update signal value directly to simulate data passing
    component.metrics.set(mockMetrics);
    fixture.detectChanges();

    expect(component.metrics()).toEqual(mockMetrics);
    expect(component.metrics()?.dominantEmotion).toBe('Joy');
  });
});
