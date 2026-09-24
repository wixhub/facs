#include <emscripten/bind.h>
#include <vector>
#include <cmath>
#include <string>
#include <cstdio>
#include <algorithm>

using namespace emscripten;

/**
 * @brief Represents a 2D coordinate point on the facial landmark grid.
 */
struct Point2D
{
    float x;
    float y;
};

/**
 * @brief Represents FACS (Facial Action Coding System) Action Units intensities (0.0 to 1.0).
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
 * @brief High-performance analytical engine for micro-expression feature extraction.
 */
class MicroExpressionAnalyzer
{
public:
    MicroExpressionAnalyzer() = default;
    ~MicroExpressionAnalyzer() = default;

    /**
     * @brief Computes facial geometry and FACS Action Units from raw 2D landmark arrays.
     * @param jsLandmarks JavaScript array of objects containing {x, y} coordinates.
     * @return ExpressionMetrics Calculated telemetry and action units.
     */
    ExpressionMetrics analyzeLandmarks(const val &jsLandmarks)
    {
        std::vector<Point2D> landmarks = convertJsLandmarks(jsLandmarks);

        // Default initialized metrics
        ExpressionMetrics metrics;
        metrics.dominantEmotion = "Initializing...";
        metrics.valenceScore = 0.0f;
        metrics.arousalScore = 0.0f;
        metrics.actionUnits = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        metrics.ratios = {0.0f, 0.0f, 0.0f};

        // MediaPipe Face Mesh provides 468 or 478 points
        if (landmarks.size() < 468)
        {
            printf("C++ Engine Warning: Insufficient landmarks count = %zu (expected >= 468)\n", landmarks.size());
            metrics.dominantEmotion = "Insufficient Landmarks";
            return metrics;
        }

        // 1. Calculate geometric features using MediaPipe canonical indices
        float eyeVertical = calculateDistance(landmarks[159], landmarks[145]);
        float eyeHorizontal = calculateDistance(landmarks[33], landmarks[133]);
        metrics.ratios.eyeApertureRatio = (eyeHorizontal > 0.0f) ? (eyeVertical / eyeHorizontal) : 0.0f;

        float mouthWidth = calculateDistance(landmarks[61], landmarks[291]);
        float interOcularDist = calculateDistance(landmarks[33], landmarks[263]);
        metrics.ratios.mouthWidthRatio = (interOcularDist > 0.0f) ? (mouthWidth / interOcularDist) : 0.0f;

        metrics.ratios.browFurrowDistance = calculateDistance(landmarks[107], landmarks[336]);

        // 2. Balanced mapping for FACS Action Units (normalized intensities 0.0 - 1.0)

        // AU1: Inner Brow Raiser
        float innerBrowDist = calculateDistance(landmarks[70], landmarks[300]);
        float browRatio = innerBrowDist / interOcularDist;
        metrics.actionUnits.au1_innerBrowRaiser = std::min(1.0f, std::max(0.0f, (browRatio - 0.32f) * 5.0f));

        // AU2: Outer Brow Raiser
        float leftOuterBrowDist = calculateDistance(landmarks[70], landmarks[33]);
        metrics.actionUnits.au2_outerBrowRaiser = std::min(1.0f, std::max(0.0f, (leftOuterBrowDist / interOcularDist - 0.40f) * 4.0f));

        // AU4: Brow Lowerer
        float furrowNorm = metrics.ratios.browFurrowDistance / interOcularDist;
        metrics.actionUnits.au4_browLowerer = std::min(1.0f, std::max(0.0f, (0.24f - furrowNorm) * 6.0f));

        // AU5: Upper Lid Raiser
        metrics.actionUnits.au5_upperLidRaiser = std::min(1.0f, std::max(0.0f, (metrics.ratios.eyeApertureRatio - 0.35f) * 8.0f));

        // AU6: Cheek Raiser
        float eyeSquint = 0.32f - metrics.ratios.eyeApertureRatio;
        metrics.actionUnits.au6_cheekRaiser = std::min(1.0f, std::max(0.0f, eyeSquint * 6.0f));

        // AU9: Nose Wrinkler
        float noseBridgeDist = calculateDistance(landmarks[168], landmarks[6]);
        metrics.actionUnits.au9_noseWrinkler = std::min(1.0f, std::max(0.0f, (0.15f - (noseBridgeDist / interOcularDist)) * 4.0f));

        // AU12: Lip Corner Puller (Smile) - adjusted threshold
        if (metrics.ratios.mouthWidthRatio > 0.52f)
        {
            float smileIntensity = (metrics.ratios.mouthWidthRatio - 0.52f) * 6.0f;
            metrics.actionUnits.au12_lipCornerPuller = std::min(1.0f, smileIntensity);
        }
        else
        {
            metrics.actionUnits.au12_lipCornerPuller = 0.0f;
        }

        // AU15: Lip Corner Depressor
        float mouthCornerDrop = calculateDistance(landmarks[61], landmarks[14]) / interOcularDist;
        metrics.actionUnits.au15_lipCornerDepressor = std::min(1.0f, std::max(0.0f, (0.18f - mouthCornerDrop) * 5.0f));

        // AU17: Chin Raiser
        float chinDist = calculateDistance(landmarks[18], landmarks[152]) / interOcularDist;
        metrics.actionUnits.au17_chinRaiser = std::min(1.0f, std::max(0.0f, (0.5f - chinDist) * 3.0f));

        // AU25: Lips Part
        float mouthVertical = calculateDistance(landmarks[13], landmarks[14]) / interOcularDist;
        metrics.actionUnits.au25_lipsPart = std::min(1.0f, std::max(0.0f, (mouthVertical - 0.05f) * 5.0f));

        // 3. Strict heuristic classification
        if (metrics.actionUnits.au12_lipCornerPuller > 0.4f)
        {
            metrics.dominantEmotion = "Smile / Positive Valence";
            metrics.valenceScore = 0.8f;
            metrics.arousalScore = 0.6f;
        }
        else if (metrics.actionUnits.au4_browLowerer > 0.35f || metrics.actionUnits.au15_lipCornerDepressor > 0.3f)
        {
            metrics.dominantEmotion = "Concentration / Sadness";
            metrics.valenceScore = -0.4f;
            metrics.arousalScore = 0.4f;
        }
        else if (metrics.actionUnits.au5_upperLidRaiser > 0.4f)
        {
            metrics.dominantEmotion = "Surprise / Alertness";
            metrics.valenceScore = 0.3f;
            metrics.arousalScore = 0.8f;
        }
        else
        {
            metrics.dominantEmotion = "Neutral Baseline";
            metrics.valenceScore = 0.0f;
            metrics.arousalScore = 0.1f;
        }

        // Debug log output to browser console
        // printf("C++ Engine Debug -> Emotion: %s | AU12: %.3f | AU1: %.3f | AU4: %.3f\n",
        //        metrics.dominantEmotion.c_str(),
        //        metrics.actionUnits.au12_lipCornerPuller,
        //        metrics.actionUnits.au1_innerBrowRaiser,
        //        metrics.actionUnits.au4_browLowerer);

        return metrics;
    }

private:
    /**
     * @brief Computes Euclidean distance between two 2D points.
     */
    float calculateDistance(const Point2D &p1, const Point2D &p2) const
    {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Converts a JavaScript array of point objects into a C++ std::vector.
     */
    std::vector<Point2D> convertJsLandmarks(const val &jsArray) const
    {
        std::vector<Point2D> points;
        int length = jsArray["length"].as<int>();
        points.reserve(length);

        for (int i = 0; i < length; ++i)
        {
            val pt = jsArray[i];
            points.push_back({pt["x"].as<float>(),
                              pt["y"].as<float>()});
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
        .function("analyzeLandmarks", &MicroExpressionAnalyzer::analyzeLandmarks);
}