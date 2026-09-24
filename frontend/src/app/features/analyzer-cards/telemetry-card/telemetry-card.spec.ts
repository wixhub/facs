import { TestBed } from '@angular/core/testing';
import { TelemetryCard } from './telemetry-card';
import { ExpressionMetrics } from '../../../core/models/expression.model';

describe('TelemetryCard Component', () => {
  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [TelemetryCard],
    }).compileComponents();
  });

  it('should create the telemetry card component successfully', () => {
    const fixture = TestBed.createComponent(TelemetryCard);
    const component = fixture.componentInstance;

    expect(component).toBeTruthy();
  });

  it('should display initializing placeholder message when metrics input is null', () => {
    const fixture = TestBed.createComponent(TelemetryCard);
    const compiled = fixture.nativeElement as HTMLElement;

    // Set metrics input to null explicitly
    fixture.componentRef.setInput('metrics', null);
    fixture.detectChanges();

    const noDataElement = compiled.querySelector('.no-data');
    expect(noDataElement).toBeTruthy();
    expect(noDataElement?.textContent).toContain(
      'Initializing engine and awaiting facial mesh tracking',
    );
  });

  it('should correctly render expression metrics and action units when data is provided', () => {
    const fixture = TestBed.createComponent(TelemetryCard);
    const compiled = fixture.nativeElement as HTMLElement;

    const mockMetrics: ExpressionMetrics = {
      dominantEmotion: 'Joy',
      valenceScore: 0.75,
      arousalScore: 0.85,
      actionUnits: {
        au1_innerBrowRaiser: 0.123,
        au2_outerBrowRaiser: 0.456,
        au4_browLowerer: 0.0,
        au5_upperLidRaiser: 0.0,
        au6_cheekRaiser: 0.889,
        au9_noseWrinkler: 0.0,
        au12_lipCornerPuller: 0.95,
        au15_lipCornerDepressor: 0.0,
        au17_chinRaiser: 0.0,
        au25_lipsPart: 0.333,
      },
      ratios: {
        eyeApertureRatio: 0.5,
        mouthWidthRatio: 0.6,
        browFurrowDistance: 0.7,
      },
    };

    // Pass mock data through the Angular component input signal
    fixture.componentRef.setInput('metrics', mockMetrics);
    fixture.detectChanges();

    // Verify dominant emotion rendering
    const highlightValue = compiled.querySelector('.metric-value.highlight');
    expect(highlightValue?.textContent?.trim()).toBe('Joy');

    // Verify valence score rendering (should include '+' for positive values)
    const metricRows = compiled.querySelectorAll('.metric-row');
    expect(metricRows.length).toBeGreaterThan(0);

    // Verify Action Units rendering (checking specific AU12 value formatting)
    const auValues = compiled.querySelectorAll('.au-val');
    expect(auValues.length).toBeGreaterThan(0);

    // Find AU12 value element and check its formatted output
    const au12Text = Array.from(auValues).find((el) => el.textContent?.trim() === '0.950');
    expect(au12Text).toBeTruthy();
  });
});
