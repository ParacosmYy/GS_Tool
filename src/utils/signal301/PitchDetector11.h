/**
 * @file PitchDetector11.h
 * @brief 基频检测器(自相关锐化与抛物线插值实现音乐信号高精度基频估计) — Pitch Detector with Autocorrelation Sharpening and Parabolic Interpolation for High-Accuracy Fundamental Frequency Estimation in Music Signals
 *
 * 功能: 实现基频检测器(Pitch detector)，采用自相关锐化(autocorrelation sharpening)
 *       与抛物线插值(parabolic interpolation)实现音乐信号高精度基频估计(high-accuracy fundamental frequency estimation in music signals)。
 *
 * 协作: Compressor10(压缩器) / SlidingDFT12(滑动DFT) / Autocorrelation(自相关)
 */
#pragma once

#include <QObject>
#include <QVector>

class PitchDetector11 : public QObject {
    Q_OBJECT

public:
    /** @brief Pitch detection result */
    struct PitchResult {
        double frequencyHz = 0.0;
        double confidence = 0.0;      // 0..1
        double clarity = 0.0;         // harmonicity measure
        bool voiced = false;
        int lagSamples = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDetections = 0;
        double lastPitchHz = 0.0;
        double minPitchHz = 1e6;
        double maxPitchHz = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector11(QObject *parent = nullptr);
    ~PitchDetector11() override;

    void setSampleRate(double rate);
    void setFrequencyRange(double minHz, double maxHz);
    void setConfidenceThreshold(double threshold);

    /** @brief Detect pitch from a frame of audio samples */
    PitchResult detect(const QVector<double>& frame);

    /** @brief Detect pitch for multiple overlapping frames */
    QVector<PitchResult> detectContinuous(const QVector<double>& audio, int frameSize, int hopSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double freqHz, double confidence, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_minHz = 50.0;
    double m_maxHz = 2000.0;
    double m_confThreshold = 0.3;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute normalized autocorrelation function (NACF) */
    QVector<double> autocorrelation(const QVector<double>& frame, int maxLag) const;

    /** @brief Apply NSDF (Normalized Square Difference Function) sharpening */
    QVector<double> sharpenACF(const QVector<double>& acf) const;

    /** @brief Find first dominant peak in sharpened ACF */
    int findPeakLag(const QVector<double>& nacf, int minLag, int maxLag) const;

    /** @brief Parabolic interpolation around peak for sub-sample accuracy */
    double parabolicInterpolation(const QVector<double>& nacf, int peakIdx) const;

    /** @brief Compute clarity (peak prominence) */
    double computeClarity(const QVector<double>& nacf, int peakIdx) const;

    /** @brief Apply center-clipping to enhance periodicity */
    QVector<double> centerClip(const QVector<double>& frame, double clipLevel) const;
};
