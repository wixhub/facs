/**
 * @brief Represents a 2D coordinate point on the facial landmark grid.
 */
export interface Point2D {
  x: number;
  y: number;
}

/**
 * @brief Calculated FACS (Facial Action Coding System) derived metrics and emotion classification.
 */
export interface ExpressionMetrics {
  dominantEmotion: string;
  valenceScore: number; // -1.0 to 1.0
  arousalScore: number; // 0.0 to 1.0
  actionUnits: {
    au1_innerBrowRaiser: number;
    au2_outerBrowRaiser: number;
    au4_browLowerer: number;
    au5_upperLidRaiser: number;
    au6_cheekRaiser: number;
    au9_noseWrinkler: number;
    au12_lipCornerPuller: number;
    au15_lipCornerDepressor: number;
    au17_chinRaiser: number;
    au25_lipsPart: number;
  };
  ratios: {
    eyeApertureRatio: number;
    mouthWidthRatio: number;
    browFurrowDistance: number;
  };
}

/**
 * @brief Emscripten module definition container.
 */
export interface EmscriptenModule {
  [key: string]: any;
}
