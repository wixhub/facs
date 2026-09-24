#include <emscripten/bind.h>
#include <vector>
#include <cmath>
#include <string>
#include <cstdio>
#include <algorithm>

using namespace emscripten;

// ==========================================
// CONFIGURATION & CONSTANTS
// ==========================================
namespace Config
{
    // MediaPipe Face Mesh structural specifications
    constexpr size_t EXPECTED_LANDMARKS_COUNT = 468;

    // Calibration settings for cross-user adaptability
    namespace Calibration
    {
        constexpr int TARGET_FRAMES = 45; // Number of initial frames to compute neutral baseline
    }

    // MediaPipe Canonical Landmark Indices
    namespace Landmarks
    {
        constexpr int LEFT_EYE_TOP = 159;
        constexpr int LEFT_EYE_BOTTOM = 145;
        constexpr int LEFT_EYE_OUTER = 33;
        constexpr int LEFT_EYE_INNER = 133;
        constexpr int RIGHT_EYE_INNER = 362;
        constexpr int RIGHT_EYE_OUTER = 263;

        constexpr int MOUTH_LEFT_CORNER = 61;
        constexpr int MOUTH_RIGHT_CORNER = 291;
        constexpr int MOUTH_TOP_CENTER = 13;
        constexpr int MOUTH_BOTTOM_CENTER = 14;
        constexpr int MOUTH_LOWER_CORNER_REF = 14;

        constexpr int BROW_INNER_LEFT = 70;
        constexpr int BROW_INNER_RIGHT = 300;
        constexpr int BROW_FURROW_LEFT = 107;
        constexpr int BROW_FURROW_RIGHT = 336;
        constexpr int NOSE_BRIDGE = 168;
        constexpr int NOSE_TIP = 6;
        constexpr int CHIN_TOP = 18;
        constexpr int CHIN_BOTTOM = 152;
    }

    // Action Units (FACS) Relative Sensitivity Scaling Constants
    namespace Thresholds
    {
        constexpr float BROW_RATIO_SCALE = 8.0f;
        constexpr float OUTER_BROW_SCALE = 6.0f;
        constexpr float BROW_FURROW_SCALE = 8.0f;
        constexpr float EYE_APERTURE_SCALE = 10.0f;
        constexpr float EYE_SQUINT_SCALE = 8.0f;
        constexpr float NOSE_BRIDGE_SCALE = 6.0f;
        constexpr float MOUTH_WIDTH_SCALE = 10.0f;
        constexpr float MOUTH_CORNER_DROP_SCALE = 8.0f;
        constexpr float CHIN_RAISE_SCALE = 5.0f;
        constexpr float LIPS_PART_SCALE = 8.0f;
    }

    // Emotion Classification Delta Thresholds (activation over baseline)
    namespace Emotions
    {
        constexpr float SMILE_ACTIVATION_LIMIT = 0.25f;
        constexpr float CONCENTRATION_ACTIVATION_LIMIT = 0.22f;
        constexpr float SADNESS_ACTIVATION_LIMIT = 0.20f;
        constexpr float SURPRISE_ACTIVATION_LIMIT = 0.25f;
    }

    // Temporal Smoothing Parameters
    namespace Smoothing
    {
        constexpr float EMA_ALPHA = 0.25f; // Smoothing factor (lower = smoother)
    }
}

/**
 * @brief Represents a 2D coordinate point on the facial landmark grid.
 */
struct Point2D
{
    float x;
    float y;
};

/**
 * @brief Represents FACS Action Units intensities (0.0 to 1.0).
 */
struct ActionUnits
{
    float au1_innerBrowRaiser;
    float au2_outerBrowRaiser;
    float au4_browLowerer;
    float au5_upperLidRaiser;
    float au6_cheekRaiser;
    float au9_noseWrinkler;
    float au12_lipCornerPuller;
    float au15_lipCornerDepressor;
    float au17_chinRaiser;
    float au25_lipsPart;
};

/**
 * @brief Container for facial geometric ratios.
 */
struct ExpressionRatios
{
    float eyeApertureRatio;
    float mouthWidthRatio;
    float browFurrowDistance;
};

/**
 * @brief Calculated FACS derived metrics, Action Units, and emotion classification.
 */
struct ExpressionMetrics
{
    std::string dominantEmotion;
    float valenceScore;
    float arousalScore;
    ActionUnits actionUnits;
    ExpressionRatios ratios;
};

/**
 * @brief High-performance analytical engine for micro-expression feature extraction
 * featuring adaptive baseline calibration and temporal smoothing (EMA).
 */
class MicroExpressionAnalyzer
{
public:
    MicroExpressionAnalyzer()
        : isCalibrated(false),
          calibrationFrameCount(0),
          isInitialized(false)
    {
        resetBaselineState();
    }

    ~MicroExpressionAnalyzer() = default;

    /**
     * @brief Resets the calibration state to allow recalibrating a new user or session.
     */
    void resetBaseline()
    {
        isCalibrated = false;
        calibrationFrameCount = 0;
        accumEyeAperture = 0.0f;
        accumMouthWidth = 0.0f;
        accumBrowFurrow = 0.0f;
        accumInnerBrow = 0.0f;
        accumOuterBrow = 0.0f;
        accumNoseBridge = 0.0f;
        accumCornerDrop = 0.0f;
        accumChinDist = 0.0f;
        accumMouthVert = 0.0f;
        isInitialized = false;
        printf("C++ Engine Info: Baseline calibration reset successfully.\n");
    }

    /**
     * @brief Computes facial geometry, adapts to user baseline, calculates FACS Action Units, and smooths output.
     * @param jsFloat32Array JavaScript Float32Array containing interleaved [x0, y0, x1, y1, ...] coordinates.
     * @return ExpressionMetrics Calculated telemetry and action units.
     */
    ExpressionMetrics analyzeLandmarks(const val &jsFloat32Array)
    {
        std::vector<Point2D> landmarks = convertJsLandmarksFast(jsFloat32Array);

        ExpressionMetrics rawMetrics;
        rawMetrics.valenceScore = 0.0f;
        rawMetrics.arousalScore = 0.0f;
        rawMetrics.actionUnits = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        rawMetrics.ratios = {0.0f, 0.0f, 0.0f};

        if (landmarks.size() < Config::EXPECTED_LANDMARKS_COUNT)
        {
            rawMetrics.dominantEmotion = "Insufficient Landmarks";
            return rawMetrics;
        }

        using namespace Config::Landmarks;
        using namespace Config::Thresholds;

        // 1. Calculate raw geometric features using canonical indices
        float eyeVertical = calculateDistance(landmarks[LEFT_EYE_TOP], landmarks[LEFT_EYE_BOTTOM]);
        float eyeHorizontal = calculateDistance(landmarks[LEFT_EYE_OUTER], landmarks[LEFT_EYE_INNER]);
        float currentEyeAperture = (eyeHorizontal > 0.0f) ? (eyeVertical / eyeHorizontal) : 0.0f;

        float mouthWidth = calculateDistance(landmarks[MOUTH_LEFT_CORNER], landmarks[MOUTH_RIGHT_CORNER]);
        float interOcularDist = calculateDistance(landmarks[LEFT_EYE_OUTER], landmarks[RIGHT_EYE_OUTER]);
        float currentMouthWidthRatio = (interOcularDist > 0.0f) ? (mouthWidth / interOcularDist) : 0.0f;

        float currentBrowFurrow = calculateDistance(landmarks[BROW_FURROW_LEFT], landmarks[BROW_FURROW_RIGHT]);
        float currentInnerBrowRatio = calculateDistance(landmarks[BROW_INNER_LEFT], landmarks[BROW_INNER_RIGHT]) / interOcularDist;
        float currentOuterBrowDist = calculateDistance(landmarks[BROW_INNER_LEFT], landmarks[LEFT_EYE_OUTER]) / interOcularDist;
        float currentNoseBridgeDist = calculateDistance(landmarks[NOSE_BRIDGE], landmarks[NOSE_TIP]) / interOcularDist;
        float currentMouthCornerDrop = calculateDistance(landmarks[MOUTH_LEFT_CORNER], landmarks[MOUTH_LOWER_CORNER_REF]) / interOcularDist;
        float currentChinDist = calculateDistance(landmarks[CHIN_TOP], landmarks[CHIN_BOTTOM]) / interOcularDist;
        float currentMouthVertical = calculateDistance(landmarks[MOUTH_TOP_CENTER], landmarks[MOUTH_BOTTOM_CENTER]) / interOcularDist;

        rawMetrics.ratios.eyeApertureRatio = currentEyeAperture;
        rawMetrics.ratios.mouthWidthRatio = currentMouthWidthRatio;
        rawMetrics.ratios.browFurrowDistance = currentBrowFurrow;

        // 2. Adaptive Calibration Phase
        if (!isCalibrated)
        {
            accumulateBaseline(currentEyeAperture, currentMouthWidthRatio, currentBrowFurrow,
                               currentInnerBrowRatio, currentOuterBrowDist, currentNoseBridgeDist,
                               currentMouthCornerDrop, currentChinDist, currentMouthVertical);

            rawMetrics.dominantEmotion = "Calibrating baseline...";
            return rawMetrics;
        }

        // 3. Map FACS Action Units intensities relative to personal baseline (Delta calculation)
        auto &au = rawMetrics.actionUnits;

        float innerBrowDelta = currentInnerBrowRatio - baselineInnerBrowRatio;
        au.au1_innerBrowRaiser = std::min(1.0f, std::max(0.0f, innerBrowDelta * BROW_RATIO_SCALE));

        float outerBrowDelta = currentOuterBrowDist - baselineOuterBrowDist;
        au.au2_outerBrowRaiser = std::min(1.0f, std::max(0.0f, outerBrowDelta * OUTER_BROW_SCALE));

        float furrowDelta = baselineRatios.browFurrowDistance - currentBrowFurrow;
        au.au4_browLowerer = std::min(1.0f, std::max(0.0f, (furrowDelta / interOcularDist) * BROW_FURROW_SCALE));

        float eyeApertureDelta = currentEyeAperture - baselineRatios.eyeApertureRatio;
        au.au5_upperLidRaiser = std::min(1.0f, std::max(0.0f, eyeApertureDelta * EYE_APERTURE_SCALE));

        float eyeSquintDelta = baselineRatios.eyeApertureRatio - currentEyeAperture;
        au.au6_cheekRaiser = std::min(1.0f, std::max(0.0f, eyeSquintDelta * EYE_SQUINT_SCALE));

        float noseDelta = baselineNoseBridgeDist - currentNoseBridgeDist;
        au.au9_noseWrinkler = std::min(1.0f, std::max(0.0f, noseDelta * NOSE_BRIDGE_SCALE));

        float mouthWidthDelta = currentMouthWidthRatio - baselineRatios.mouthWidthRatio;
        au.au12_lipCornerPuller = std::min(1.0f, std::max(0.0f, mouthWidthDelta * MOUTH_WIDTH_SCALE));

        float cornerDropDelta = currentMouthCornerDrop - baselineMouthCornerDrop;
        au.au15_lipCornerDepressor = std::min(1.0f, std::max(0.0f, cornerDropDelta * MOUTH_CORNER_DROP_SCALE));

        float chinDelta = baselineChinDist - currentChinDist;
        au.au17_chinRaiser = std::min(1.0f, std::max(0.0f, chinDelta * CHIN_RAISE_SCALE));

        float lipsPartDelta = currentMouthVertical - baselineMouthVertical;
        au.au25_lipsPart = std::min(1.0f, std::max(0.0f, lipsPartDelta * LIPS_PART_SCALE));

        // 4. Heuristic emotion classification based on dynamic deltas
        using namespace Config::Emotions;
        if (au.au12_lipCornerPuller > SMILE_ACTIVATION_LIMIT)
        {
            rawMetrics.dominantEmotion = "Smile / Positive Valence";
            rawMetrics.valenceScore = 0.8f;
            rawMetrics.arousalScore = 0.6f;
        }
        else if (au.au4_browLowerer > CONCENTRATION_ACTIVATION_LIMIT || au.au15_lipCornerDepressor > SADNESS_ACTIVATION_LIMIT)
        {
            rawMetrics.dominantEmotion = "Concentration / Sadness";
            rawMetrics.valenceScore = -0.4f;
            rawMetrics.arousalScore = 0.4f;
        }
        else if (au.au5_upperLidRaiser > SURPRISE_ACTIVATION_LIMIT)
        {
            rawMetrics.dominantEmotion = "Surprise / Alertness";
            rawMetrics.valenceScore = 0.3f;
            rawMetrics.arousalScore = 0.8f;
        }
        else
        {
            rawMetrics.dominantEmotion = "Neutral Baseline";
            rawMetrics.valenceScore = 0.0f;
            rawMetrics.arousalScore = 0.1f;
        }

        // 5. Temporal Smoothing (Exponential Moving Average - EMA)
        const float alpha = Config::Smoothing::EMA_ALPHA;

        if (!isInitialized)
        {
            smoothedMetrics = rawMetrics;
            isInitialized = true;
        }
        else
        {
            smoothedMetrics.ratios.eyeApertureRatio = alpha * rawMetrics.ratios.eyeApertureRatio + (1.0f - alpha) * smoothedMetrics.ratios.eyeApertureRatio;
            smoothedMetrics.ratios.mouthWidthRatio = alpha * rawMetrics.ratios.mouthWidthRatio + (1.0f - alpha) * smoothedMetrics.ratios.mouthWidthRatio;
            smoothedMetrics.ratios.browFurrowDistance = alpha * rawMetrics.ratios.browFurrowDistance + (1.0f - alpha) * smoothedMetrics.ratios.browFurrowDistance;

            auto &sAU = smoothedMetrics.actionUnits;
            const auto &rAU = rawMetrics.actionUnits;
            sAU.au1_innerBrowRaiser = alpha * rAU.au1_innerBrowRaiser + (1.0f - alpha) * sAU.au1_innerBrowRaiser;
            sAU.au2_outerBrowRaiser = alpha * rAU.au2_outerBrowRaiser + (1.0f - alpha) * sAU.au2_outerBrowRaiser;
            sAU.au4_browLowerer = alpha * rAU.au4_browLowerer + (1.0f - alpha) * sAU.au4_browLowerer;
            sAU.au5_upperLidRaiser = alpha * rAU.au5_upperLidRaiser + (1.0f - alpha) * sAU.au5_upperLidRaiser;
            sAU.au6_cheekRaiser = alpha * rAU.au6_cheekRaiser + (1.0f - alpha) * sAU.au6_cheekRaiser;
            sAU.au9_noseWrinkler = alpha * rAU.au9_noseWrinkler + (1.0f - alpha) * sAU.au9_noseWrinkler;
            sAU.au12_lipCornerPuller = alpha * rAU.au12_lipCornerPuller + (1.0f - alpha) * sAU.au12_lipCornerPuller;
            sAU.au15_lipCornerDepressor = alpha * rAU.au15_lipCornerDepressor + (1.0f - alpha) * sAU.au15_lipCornerDepressor;
            sAU.au17_chinRaiser = alpha * rAU.au17_chinRaiser + (1.0f - alpha) * sAU.au17_chinRaiser;
            sAU.au25_lipsPart = alpha * rAU.au25_lipsPart + (1.0f - alpha) * sAU.au25_lipsPart;

            smoothedMetrics.valenceScore = alpha * rawMetrics.valenceScore + (1.0f - alpha) * smoothedMetrics.valenceScore;
            smoothedMetrics.arousalScore = alpha * rawMetrics.arousalScore + (1.0f - alpha) * smoothedMetrics.arousalScore;
            smoothedMetrics.dominantEmotion = rawMetrics.dominantEmotion;
        }

        return smoothedMetrics;
    }

private:
    bool isCalibrated;
    int calibrationFrameCount;
    ExpressionRatios baselineRatios;
    float baselineInnerBrowRatio;
    float baselineOuterBrowDist;
    float baselineNoseBridgeDist;
    float baselineMouthCornerDrop;
    float baselineChinDist;
    float baselineMouthVertical;

    float accumEyeAperture;
    float accumMouthWidth;
    float accumBrowFurrow;
    float accumInnerBrow;
    float accumOuterBrow;
    float accumNoseBridge;
    float accumCornerDrop;
    float accumChinDist;
    float accumMouthVert;

    ExpressionMetrics smoothedMetrics;
    bool isInitialized;

    void resetBaselineState()
    {
        baselineRatios = {0.0f, 0.0f, 0.0f};
        baselineInnerBrowRatio = 0.0f;
        baselineOuterBrowDist = 0.0f;
        baselineNoseBridgeDist = 0.0f;
        baselineMouthCornerDrop = 0.0f;
        baselineChinDist = 0.0f;
        baselineMouthVertical = 0.0f;

        accumEyeAperture = 0.0f;
        accumMouthWidth = 0.0f;
        accumBrowFurrow = 0.0f;
        accumInnerBrow = 0.0f;
        accumOuterBrow = 0.0f;
        accumNoseBridge = 0.0f;
        accumCornerDrop = 0.0f;
        accumChinDist = 0.0f;
        accumMouthVert = 0.0f;
    }

    void accumulateBaseline(float eyeApt, float mouthW, float furrow, float innerBrow,
                            float outerBrow, float nose, float cornerDrop, float chin, float mouthVert)
    {
        accumEyeAperture += eyeApt;
        accumMouthWidth += mouthW;
        accumBrowFurrow += furrow;
        accumInnerBrow += innerBrow;
        accumOuterBrow += outerBrow;
        accumNoseBridge += nose;
        accumCornerDrop += cornerDrop;
        accumChinDist += chin;
        accumMouthVert += mouthVert;

        calibrationFrameCount++;

        if (calibrationFrameCount >= Config::Calibration::TARGET_FRAMES)
        {
            float floatFrames = static_cast<float>(Config::Calibration::TARGET_FRAMES);

            baselineRatios.eyeApertureRatio = accumEyeAperture / floatFrames;
            baselineRatios.mouthWidthRatio = accumMouthWidth / floatFrames;
            baselineRatios.browFurrowDistance = accumBrowFurrow / floatFrames;

            baselineInnerBrowRatio = accumInnerBrow / floatFrames;
            baselineOuterBrowDist = accumOuterBrow / floatFrames;
            baselineNoseBridgeDist = accumNoseBridge / floatFrames;
            baselineMouthCornerDrop = accumCornerDrop / floatFrames;
            baselineChinDist = accumChinDist / floatFrames;
            baselineMouthVertical = accumMouthVert / floatFrames;

            isCalibrated = true;
            printf("C++ Engine Info: Adaptive baseline calibration completed successfully.\n");
        }
    }

    float calculateDistance(const Point2D &p1, const Point2D &p2) const
    {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    std::vector<Point2D> convertJsLandmarksFast(const val &jsFloat32Array) const
    {
        int length = jsFloat32Array["length"].as<int>();
        int numPoints = length / 2;

        std::vector<Point2D> points;
        points.reserve(numPoints);

        for (int i = 0; i < length; i += 2)
        {
            points.push_back({jsFloat32Array[i].as<float>(),
                              jsFloat32Array[i + 1].as<float>()});
        }

        return points;
    }
};

// ==========================================
// EMSCRIPTEN BINDINGS EXPORT TO WEBASSEMBLY
// ==========================================
EMSCRIPTEN_BINDINGS(MicroExpressionEngineModule)
{
    value_object<Point2D>("Point2D")
        .field("x", &Point2D::x)
        .field("y", &Point2D::y);

    value_object<ActionUnits>("ActionUnits")
        .field("au1_innerBrowRaiser", &ActionUnits::au1_innerBrowRaiser)
        .field("au2_outerBrowRaiser", &ActionUnits::au2_outerBrowRaiser)
        .field("au4_browLowerer", &ActionUnits::au4_browLowerer)
        .field("au5_upperLidRaiser", &ActionUnits::au5_upperLidRaiser)
        .field("au6_cheekRaiser", &ActionUnits::au6_cheekRaiser)
        .field("au9_noseWrinkler", &ActionUnits::au9_noseWrinkler)
        .field("au12_lipCornerPuller", &ActionUnits::au12_lipCornerPuller)
        .field("au15_lipCornerDepressor", &ActionUnits::au15_lipCornerDepressor)
        .field("au17_chinRaiser", &ActionUnits::au17_chinRaiser)
        .field("au25_lipsPart", &ActionUnits::au25_lipsPart);

    value_object<ExpressionRatios>("ExpressionRatios")
        .field("eyeApertureRatio", &ExpressionRatios::eyeApertureRatio)
        .field("mouthWidthRatio", &ExpressionRatios::mouthWidthRatio)
        .field("browFurrowDistance", &ExpressionRatios::browFurrowDistance);

    value_object<ExpressionMetrics>("ExpressionMetrics")
        .field("dominantEmotion", &ExpressionMetrics::dominantEmotion)
        .field("valenceScore", &ExpressionMetrics::valenceScore)
        .field("arousalScore", &ExpressionMetrics::arousalScore)
        .field("actionUnits", &ExpressionMetrics::actionUnits)
        .field("ratios", &ExpressionMetrics::ratios);

    class_<MicroExpressionAnalyzer>("MicroExpressionAnalyzer")
        .constructor<>()
        .function("analyzeLandmarks", &MicroExpressionAnalyzer::analyzeLandmarks)
        .function("resetBaseline", &MicroExpressionAnalyzer::resetBaseline); // Экспорт метода в JS/TS
}