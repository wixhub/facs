import {
  Component,
  ElementRef,
  OnInit,
  ViewChild,
  inject,
  signal,
  OnDestroy,
  output,
} from '@angular/core';
import { FaceLandmarker, FilesetResolver } from '@mediapipe/tasks-vision';
import { ExpressionMetrics } from '../../../core/models/expression.model';
import { WasmService } from '../../../core/services/wasm.service';

@Component({
  imports: [],
  selector: 'app-video-card',
  styleUrl: './video-card.scss',
  templateUrl: './video-card.html',
})
export class VideoCard implements OnInit, OnDestroy {
  private wasmService = inject(WasmService);

  @ViewChild('videoElement', { static: true }) videoElement!: ElementRef<HTMLVideoElement>;

  // Output for parent
  readonly metricsChange = output<ExpressionMetrics | null>();

  // Reactive signals for component state
  readonly activeSource = signal<'webcam' | 'demo'>('webcam');
  readonly isTracking = signal<boolean>(false);
  readonly fps = signal<number>(0);

  // Signal to store current telemetry metrics (used for calibration overlay in HTML)
  readonly currentMetrics = signal<ExpressionMetrics | null>(null);

  private mediaStream: MediaStream | null = null;
  private animationFrameId: number | null = null;
  private faceLandmarker: FaceLandmarker | null = null;

  async ngOnInit(): Promise<void> {
    // 1. Initialize MediaPipe Face Landmarker model first
    await this.initMediaPipe();

    // 2. Start default video source (demo)
    await this.setSource('demo');
  }

  ngOnDestroy(): void {
    this.stopTracking();
  }

  /**
   * @brief Initializes MediaPipe FaceLandmarker tasks-vision from CDN and loads the model.
   */
  private async initMediaPipe(): Promise<void> {
    try {
      const vision = await FilesetResolver.forVisionTasks(
        'https://cdn.jsdelivr.net/npm/@mediapipe/tasks-vision@latest/wasm',
      );
      this.faceLandmarker = await FaceLandmarker.createFromOptions(vision, {
        baseOptions: {
          modelAssetPath:
            'https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/1/face_landmarker.task',
          delegate: 'GPU',
        },
        runningMode: 'VIDEO',
        numFaces: 1,
      });
    } catch (err) {
      console.error('Failed to initialize MediaPipe FaceLandmarker:', err);
    }
  }

  /**
   * @brief Switches the video data source between live webcam and pre-recorded demo video.
   * @param source Target source type ('webcam' or 'demo').
   */
  async setSource(source: 'webcam' | 'demo'): Promise<void> {
    if (this.activeSource() === source && this.isTracking()) return;

    this.activeSource.set(source);
    this.currentMetrics.set(null);

    if (source === 'webcam') {
      this.wasmService.resetBaseline(); // Triggers calibration only for webcam
    }

    this.stopTracking();
    await this.initializeVideoSource(source);
  }

  /**
   * @brief Configures media streams or HTML5 video element source.
   */
  private async initializeVideoSource(source: 'webcam' | 'demo'): Promise<void> {
    const video = this.videoElement.nativeElement;

    if (source === 'webcam') {
      try {
        this.mediaStream = await navigator.mediaDevices.getUserMedia({
          video: { width: 640, height: 480, facingMode: 'user' },
        });
        video.srcObject = this.mediaStream;
        video.src = '';
        video.loop = false;
        await video.play();
        this.startAnalysisLoop();
      } catch (err) {
        console.error('Error accessing webcam hardware:', err);
      }
    } else {
      // Clear webcam stream if active
      if (this.mediaStream) {
        this.mediaStream.getTracks().forEach((track) => track.stop());
        this.mediaStream = null;
      }

      video.srcObject = null;
      video.src = '/sample.mp4'; // Loaded from the public directory
      video.loop = true;
      video.muted = true;

      try {
        await video.play();
        this.startAnalysisLoop();
      } catch (err) {
        console.error('Error playing demo video stream:', err);
      }
    }
  }

  /**
   * @brief Starts the high-performance frame processing and telemetry analysis loop.
   */
  private startAnalysisLoop(): void {
    this.isTracking.set(true);
    let lastTime = performance.now();
    let frameCount = 0;

    const loop = () => {
      if (!this.isTracking()) return;

      const video = this.videoElement.nativeElement;
      if (video.readyState >= 2 && this.faceLandmarker) {
        // Extract 468 landmark points using MediaPipe
        const results = this.faceLandmarker.detectForVideo(video, performance.now());

        if (results.faceLandmarks && results.faceLandmarks.length > 0) {
          const landmarks = results.faceLandmarks[0];

          // Pass extracted landmarks into C++ WebAssembly engine
          const telemetry = this.wasmService.analyzeLandmarks(landmarks);

          // Store in signal and broadcast telemetry to parent component
          this.currentMetrics.set(telemetry);
          this.metricsChange.emit(telemetry);
        } else {
          this.currentMetrics.set(null);
          this.metricsChange.emit(null);
        }

        // Calculate real-time FPS
        frameCount++;
        const now = performance.now();
        if (now - lastTime >= 1000) {
          this.fps.set(frameCount);
          frameCount = 0;
          lastTime = now;
        }
      }

      this.animationFrameId = requestAnimationFrame(loop);
    };

    this.animationFrameId = requestAnimationFrame(loop);
  }

  /**
   * @brief Safely stops the analysis loop and releases hardware streams.
   */
  private stopTracking(): void {
    this.isTracking.set(false);
    if (this.animationFrameId) {
      cancelAnimationFrame(this.animationFrameId);
      this.animationFrameId = null;
    }
    if (this.mediaStream) {
      this.mediaStream.getTracks().forEach((track) => track.stop());
      this.mediaStream = null;
    }
  }
}
