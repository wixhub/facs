import { TestBed } from '@angular/core/testing';
import { WasmService } from './wasm.service';

describe('WasmService', () => {
  let service: WasmService;

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [WasmService],
    });
    service = TestBed.inject(WasmService);
  });

  afterEach(() => {
    vi.clearAllMocks();
  });

  it('should be created successfully', () => {
    // Assert that the service instance is correctly initialized
    expect(service).toBeTruthy();
  });

  it('should return null when analyzing landmarks before the WASM module is loaded', () => {
    // When the resource hasn't resolved yet, analyzing should safely return null
    const mockLandmarks = [{ x: 0.5, y: 0.5 }];
    const result = service.analyzeLandmarks(mockLandmarks);

    expect(result).toBeNull();
  });

  it('should correctly process landmarks and return mapped expression metrics when loaded', () => {
    // Create a mock C++ analyzer instance
    const analyzerMock = {
      analyzeLandmarks: vi.fn().mockReturnValue({
        dominantEmotion: 'Surprise',
        valenceScore: 0.4,
        arousalScore: 0.9,
        actionUnits: {
          au1_innerBrowRaiser: 0.9,
          au2_outerBrowRaiser: 0.8,
          au4_browLowerer: 0.1,
          au5_upperLidRaiser: 0.8,
          au6_cheekRaiser: 0.2,
          au9_noseWrinkler: 0.0,
          au12_lipCornerPuller: 0.1,
          au15_lipCornerDepressor: 0.0,
          au17_chinRaiser: 0.0,
          au25_lipsPart: 0.7,
        },
        ratios: {
          eyeApertureRatio: 0.8,
          mouthWidthRatio: 0.5,
          browFurrowDistance: 0.4,
        },
      }),
    };

    // Spy on resource value and isLoaded computed signal
    vi.spyOn(service.wasmResource, 'value').mockReturnValue(analyzerMock as any);
    vi.spyOn(service, 'isLoaded').mockReturnValue(true);

    const mockLandmarks = [
      { x: 0.1, y: 0.2 },
      { x: 0.3, y: 0.4 },
    ];

    const metrics = service.analyzeLandmarks(mockLandmarks);

    // Verify mapping from C++ raw output to Angular model
    expect(metrics).not.toBeNull();
    expect(metrics?.dominantEmotion).toBe('Surprise');
    expect(metrics?.valenceScore).toBe(0.4);
    expect(metrics?.actionUnits.au1_innerBrowRaiser).toBe(0.9);
    expect(metrics?.ratios.eyeApertureRatio).toBe(0.8);
    expect(analyzerMock.analyzeLandmarks).toHaveBeenCalledWith(mockLandmarks);
  });

  it('should handle errors gracefully during landmark analysis', () => {
    const faultyAnalyzerMock = {
      analyzeLandmarks: vi.fn().mockImplementation(() => {
        throw new Error('C++ memory access violation');
      }),
    };

    vi.spyOn(service.wasmResource, 'value').mockReturnValue(faultyAnalyzerMock as any);
    vi.spyOn(service, 'isLoaded').mockReturnValue(true);

    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
    const result = service.analyzeLandmarks([{ x: 0, y: 0 }]);

    expect(result).toBeNull();
    expect(consoleSpy).toHaveBeenCalled();
    consoleSpy.mockRestore();
  });
});
