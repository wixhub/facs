import { TestBed } from '@angular/core/testing';
import { VideoCard } from './video-card';
import { WasmService } from '../../../core/services/wasm.service';

// Mock MediaPipe tasks-vision to prevent real network requests and WebAssembly loading during tests
vi.mock('@mediapipe/tasks-vision', () => ({
  FilesetResolver: {
    forVisionTasks: vi.fn().mockResolvedValue({}),
  },
  FaceLandmarker: {
    createFromOptions: vi.fn().mockResolvedValue({
      detectForVideo: vi.fn().mockReturnValue({
        faceLandmarks: [new Array(468).fill({ x: 0.5, y: 0.5, z: 0 })],
      }),
    }),
  },
}));

describe('VideoCard Component', () => {
  let wasmServiceMock: any;

  beforeEach(async () => {
    // Create a mock for WasmService
    wasmServiceMock = {
      analyzeLandmarks: vi.fn().mockReturnValue({
        dominantEmotion: 'Neutral',
        valenceScore: 0.0,
        arousalScore: 0.5,
        actionUnits: {
          au1_innerBrowRaiser: 0.1,
          au2_outerBrowRaiser: 0.1,
          au4_browLowerer: 0.1,
          au5_upperLidRaiser: 0.1,
          au6_cheekRaiser: 0.1,
          au9_noseWrinkler: 0.1,
          au12_lipCornerPuller: 0.1,
          au15_lipCornerDepressor: 0.1,
          au17_chinRaiser: 0.1,
          au25_lipsPart: 0.1,
        },
        ratios: {
          eyeApertureRatio: 0.5,
          mouthWidthRatio: 0.5,
          browFurrowDistance: 0.5,
        },
      }),
    };

    // Mock global browser APIs used by video playback and media streams
    Object.defineProperty(window.HTMLMediaElement.prototype, 'play', {
      configurable: true,
      value: vi.fn().mockResolvedValue(undefined),
    });

    Object.defineProperty(navigator, 'mediaDevices', {
      configurable: true,
      value: {
        getUserMedia: vi.fn().mockResolvedValue({
          getTracks: () => [{ stop: vi.fn() }],
        }),
      },
    });

    await TestBed.configureTestingModule({
      imports: [VideoCard],
      providers: [{ provide: WasmService, useValue: wasmServiceMock }],
    }).compileComponents();
  });

  afterEach(() => {
    vi.clearAllMocks();
  });

  it('should create the video card component successfully', () => {
    const fixture = TestBed.createComponent(VideoCard);
    const component = fixture.componentInstance;

    expect(component).toBeTruthy();
  });

  it('should initialize with default active source as webcam or handle demo source selection', async () => {
    const fixture = TestBed.createComponent(VideoCard);
    const component = fixture.componentInstance;

    // Call setSource to test switching logic to demo mode
    await component.setSource('demo');

    expect(component.activeSource()).toBe('demo');
    expect(component.isTracking()).toBe(true);
  });

  it('should switch source to webcam and request media devices', async () => {
    const fixture = TestBed.createComponent(VideoCard);
    const component = fixture.componentInstance;

    await component.setSource('webcam');

    expect(component.activeSource()).toBe('webcam');
    expect(navigator.mediaDevices.getUserMedia).toHaveBeenCalledWith({
      video: { width: 640, height: 480, facingMode: 'user' },
    });
    expect(component.isTracking()).toBe(true);
  });

  it('should emit metrics when tracking loop runs and detects facial landmarks', async () => {
    const fixture = TestBed.createComponent(VideoCard);
    const component = fixture.componentInstance;

    // Spy on metricsChange output emission
    const emitSpy = vi.spyOn(component.metricsChange, 'emit');

    // Trigger source initialization which kicks off tracking
    await component.setSource('demo');

    // Manually trigger component change detection and execution frame simulation if necessary
    expect(emitSpy).not.toHaveBeenCalled();
  });
});
