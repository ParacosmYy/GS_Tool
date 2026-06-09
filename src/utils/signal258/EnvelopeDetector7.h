/**
 * @file EnvelopeDetector7.h
 * @brief 包络检测(峰值滤波器级联加权谐波强调+RMS-峰值比跟踪) — Envelope Detector with Peaking Filter Cascade for Weighted Harmonic Emphasis and RMS-to-Peak Ratio Tracking
 *
 * 功能: 实现包络检测器(Envelope detector)，通过峰值滤波器级联(peaking
 *       filter cascade)进行加权谐波强调(weighted harmonic emphasis)，
 *       并跟踪RMS-峰值比(RMS-to-peak ratio)进行包络分析。
 *
 * 协作: AMModem4(AM调制解调) / HilbertTransform5(希尔伯特变换) / PeakDetector3(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测(峰值滤波器级联+RMS-峰值比跟踪)
 */
class EnvelopeDetector7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
        double rmsToPeakRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Peaking filter stage parameters */
    struct PeakingStage {
        double centerFreq = 0.0;
        double gain = 0.0;
        double q = 1.0;
    };

    explicit EnvelopeDetector7(QObject *parent = nullptr);
    ~EnvelopeDetector7() override;

    /** @brief Set attack time in milliseconds */
    void setAttack(double attackMs);

    /** @brief Set release time in milliseconds */
    void setRelease(double releaseMs);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set peaking filter cascade stages */
    void setPeakingStages(const QVector<PeakingStage>& stages);

    /** @brief Process input and return envelope */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current RMS-to-peak ratio */
    double rmsToPeakRatio() const;

    /** @brief Get instantaneous envelope at a point */
    double envelopeAt(int index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int samples, double peak, double rmsToPeak, double timeMs);

private:
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    double m_sampleRate = 44100.0;

    // Envelope follower state
    double m_envelope = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    // Peaking filter cascade (biquad per stage)
    struct BiquadState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
    };
    QVector<BiquadState> m_filters;

    QVector<double> m_lastEnvelope;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update attack/release coefficients */
    void updateCoefficients();

    /** @brief Design biquad peaking filter for a stage */
    BiquadState designPeakingFilter(const PeakingStage& stage) const;

    /** @brief Apply peaking filter cascade to a sample */
    double applyPeakingCascade(double sample);

    /** @brief Apply envelope follower (attack/release) */
    double followEnvelope(double sample);

    /** @brief Compute RMS of envelope buffer */
    double computeRMS(const QVector<double>& env) const;
};
