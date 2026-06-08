/**
 * @file SlidingDFT6.h
 * @brief 滑动DFT(稳定Goertzel窗口+级联滑动频谱实时分析) — Sliding DFT with Guaranteed-Stability Goertzel Window and Cascaded Sliding Spectrum for Real-Time Analysis
 *
 * 功能: 实现滑动DFT，支持Goertzel窗口保证稳定性、
 *       级联滑动频谱实时分析和逐样本更新。
 *
 * 协作: ZoomFFT3(缩放FFT) / DistributedArithmetic6(分布式算术) / FFTAnalyzer(FFT分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(Goertzel窗口+级联滑动频谱)
 */
class SlidingDFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int blockSize = 0;
        int numBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT6(QObject *parent = nullptr);
    ~SlidingDFT6() override;

    /** @brief Set DFT parameters: block size and number of frequency bins */
    void setParameters(int blockSize, int numBins = 0);

    /** @brief Process a single sample and update sliding spectrum */
    void pushSample(double sample);

    /** @brief Process a block of samples */
    void processBlock(const QVector<double>& samples);

    /** @brief Get current magnitude spectrum */
    QVector<double> magnitudeSpectrum() const;

    /** @brief Get current phase spectrum */
    QVector<double> phaseSpectrum() const;

    /** @brief Get frequency axis labels */
    QVector<double> frequencyAxis(double sampleRate = 44100.0) const;

    /** @brief Reset internal state */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int bins, double timeMs);

private:
    int m_blockSize = 64;
    int m_numBins = 32;

    // Goertzel state per bin: s1, s2 (previous outputs)
    QVector<double> m_s1;
    QVector<double> m_s2;
    // Circular input buffer
    QVector<double> m_buffer;
    int m_bufferIdx = 0;
    int m_samplesProcessed = 0;

    // Window coefficients per bin
    QVector<double> m_coeff;     // 2*cos(2*pi*k/N)
    QVector<double> m_freqRatio; // k/N

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize Goertzel coefficients */
    void initCoefficients();

    /** @brief Apply stability window to prevent accumulation errors */
    void applyStabilityWindow();
};
