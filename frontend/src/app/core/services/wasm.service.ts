import { Service, resource, computed } from '@angular/core';
import { ExpressionMetrics, Point2D } from '../models/expression.model';

@Service()
export class WasmService {
  /**
   * @brief Angular resource primitive managing the asynchronous lifecycle
   * of the WebAssembly module and C++ analyzer instance.
   */
  readonly wasmResource = resource({
    loader: async (): Promise<any> => {
      // Path to the compiled Emscripten JS file served from the public directory
      const modulePath = '/wasm/analyzer.js';

      // Dynamically import the Emscripten factory module
      const emscriptenModule = await import(/* @vite-ignore */ modulePath);
      const createMicroExpressionModule = emscriptenModule.default;

      if (typeof createMicroExpressionModule !== 'function') {
        throw new Error('WASM factory function could not be loaded from analyzer.js.');
      }

      // Initialize the module and locate the corresponding .wasm binary file
      const wasmModule = await createMicroExpressionModule({
        locateFile: (path: string) => {
          if (path.endsWith('.wasm')) {
            return '/wasm/analyzer.wasm';
          }
          return path;
        },
      });

      // Return the instantiated C++ class instance stored as the resource value
      return new wasmModule.MicroExpressionAnalyzer();
    },
  });

  // Reactive signals explicitly wrapped with computed for robust template typing
  readonly isLoaded = computed(() => this.wasmResource.hasValue());
  readonly isLoading = computed(() => this.wasmResource.isLoading());
  readonly loadError = computed(() => {
    const err = this.wasmResource.error();
    return err ? (err instanceof Error ? err.message : String(err)) : null;
  });

  /**
   * @brief Resets the baseline calibration state in the C++ analyzer engine.
   */
  public resetBaseline(): void {
    const analyzerInstance = this.wasmResource.value();
    if (analyzerInstance && typeof analyzerInstance.resetBaseline === 'function') {
      try {
        analyzerInstance.resetBaseline();
      } catch (e) {
        console.error('Error resetting baseline in C++ engine:', e);
      }
    }
  }

  /**
   * @brief Analyzes facial landmarks using the C++ native engine via WebAssembly.
   * Converts object-based Point2D array into a flat Float32Array to eliminate bridge overhead.
   * @param landmarks Array of 2D points representing the facial mesh.
   * @return ExpressionMetrics Calculated FACS Action Units and emotion metrics.
   */
  public analyzeLandmarks(landmarks: Point2D[]): ExpressionMetrics | null {
    const analyzerInstance = this.wasmResource.value();

    if (!this.isLoaded() || !analyzerInstance || !landmarks || landmarks.length === 0) {
      return null;
    }

    try {
      // 1. Pack individual JS point objects into a flat typed array: [x0, y0, x1, y1, ...]
      const flatArray = new Float32Array(landmarks.length * 2);
      for (let i = 0; i < landmarks.length; i++) {
        flatArray[i * 2] = landmarks[i].x;
        flatArray[i * 2 + 1] = landmarks[i].y;
      }

      // 2. Send the fast typed array buffer to the C++ WebAssembly module
      const result = analyzerInstance.analyzeLandmarks(flatArray);

      // 3. Map C++ engine response to the extended ExpressionMetrics interface
      return {
        dominantEmotion: result.dominantEmotion ?? 'Neutral',
        valenceScore: result.valenceScore ?? 0.0,
        arousalScore: result.arousalScore ?? 0.0,
        actionUnits: {
          au1_innerBrowRaiser: result.actionUnits?.au1_innerBrowRaiser ?? 0,
          au2_outerBrowRaiser: result.actionUnits?.au2_outerBrowRaiser ?? 0,
          au4_browLowerer: result.actionUnits?.au4_browLowerer ?? 0,
          au5_upperLidRaiser: result.actionUnits?.au5_upperLidRaiser ?? 0,
          au6_cheekRaiser: result.actionUnits?.au6_cheekRaiser ?? 0,
          au9_noseWrinkler: result.actionUnits?.au9_noseWrinkler ?? 0,
          au12_lipCornerPuller: result.actionUnits?.au12_lipCornerPuller ?? 0,
          au15_lipCornerDepressor: result.actionUnits?.au15_lipCornerDepressor ?? 0,
          au17_chinRaiser: result.actionUnits?.au17_chinRaiser ?? 0,
          au25_lipsPart: result.actionUnits?.au25_lipsPart ?? 0,
        },
        ratios: {
          eyeApertureRatio: result.ratios?.eyeApertureRatio ?? 0,
          mouthWidthRatio: result.ratios?.mouthWidthRatio ?? 0,
          browFurrowDistance: result.ratios?.browFurrowDistance ?? 0,
        },
      };
    } catch (e) {
      console.error('Error during C++ landmark analysis execution:', e);
      return null;
    }
  }
}
