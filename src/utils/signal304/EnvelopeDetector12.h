/**
 * @file EnvelopeDetector12.h
 * @brief 包络检测器(小波多分辨率能量跟踪与起始检测实现打击乐瞬态分析) — Envelope Detector with Wavelet-Based Multi-Resolution Energy Tracking and Onset Detection for Percussive Transient Analysis
 *
 * 功能: 实现包络检测器(envelope detector)，采用小波多分辨率能量跟踪(wavelet-based multi-resolution energy tracking)
 *       与起始检测(onset detection)实现打击乐瞬态分析(percussive transient analysis)。
 *
 * 协作: HilbertTransform(希尔伯特变换) / WaveletTransform(小波变换) / PeakDetector(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>

class EnvelopeDetector12 : public QObject {
    Q_OBJECT

public:
    /** @brief Single wavelet decomposition level */
    struct WaveletLevel {
        QVector<double> approx;     // Approximation coefficients
        QVector<double> detail;     // Detail coefficients
        double energy = 0.0;        // Energy at this level
    };

    /** @brief Onset event */
    struct Onset {
        int sampleIndex = 0;
        double strength = 0.0;      // Detection function value
        int level = 0;              // Dominant wavelet level
    };

    /** @brief Detection result */
    struct DetectResult {
        QVector<double> envelope;
        QVector<WaveletLevel> levels;
        QVector<Onset> onsets;
        double peakEnvelope = 0.0;
        int numLevels = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDetections = 0;
        double avgOnsetsPerFrame = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector12(QObject *parent = nullptr);
    ~EnvelopeDetector12() override;

    void setDecompositionLevels(int levels);
    void setOnsetThreshold(double threshold);
    void setSampleRate(double rate);

    /** @brief Detect envelope with multi-resolution wavelet analysis */
    DetectResult detect(const QVector<double>& input);

    /** @brief Detect onsets only (lightweight) */
    QVector<Onset> detectOnsets(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionDone(int numOnsets, double peakEnvelope, double timeMs);

private:
    int m_levels = 5;
    double m_onsetThreshold = 0.3;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_onsetSum = 0.0;
    double m_timeSum = 0.0;

    // Haar wavelet coefficients
    QVector<double> m_loD;      // Low-pass decomposition
    QVector<double> m_hiD;      // High-pass decomposition

    /** @brief Initialize Haar wavelet filters */
    void initWavelet();

    /** @brief Single-level wavelet decomposition */
    WaveletLevel decompose(const QVector<double>& signal) const;

    /** @brief Compute energy at each level */
    QVector<double> computeEnergies(const QVector<WaveletLevel>& levels) const;

    /** @brief Multi-resolution envelope from wavelet coefficients */
    QVector<double> buildEnvelope(const QVector<WaveletLevel>& levels, int len) const;

    /** @brief Onset detection function (spectral flux) */
    QVector<double> onsetFunction(const QVector<WaveletLevel>& levels) const;

    /** @brief Peak picking with adaptive threshold */
    QVector<Onset> pickPeaks(const QVector<double>& func, double threshold) const;
};
