/**
 * @file Deesser5.h
 * @brief 去齿音处理器(频谱质心齿音检测+频率选择性动态扩展) — De-esser with Spectral Centroid Sibilance Detector and Frequency-Selective Dynamic Expansion
 *
 * 功能: 实现去齿音音频处理，通过频谱质心检测齿音频率范围，
 *       应用频率选择性动态扩展进行自适应增益控制。
 *
 * 协作: DynamicCompressor4(动态压缩) / Equalizer4(均衡器) / SpectralAnalyzer3(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音处理器(频谱质心检测+频率选择性扩展)
 */
class Deesser5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        double sibilanceRate = 0.0;
        double avgReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser5(QObject *parent = nullptr);
    ~Deesser5() override;

    /** @brief Set parameters: threshold, frequency range, reduction amount */
    void setParameters(double thresholdDb = -20.0,
                       double freqLow = 4000.0,
                       double freqHigh = 9000.0,
                       double reductionDb = -10.0,
                       int frameSize = 1024);

    /** @brief Process audio frame, returns de-essed output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Detect sibilance level in frame (0..1) */
    double detectSibilance(const QVector<double>& frame) const;

    /** @brief Compute spectral centroid */
    double spectralCentroid(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double sibilanceLevel, double reductionDb, double timeMs);

private:
    double m_thresholdDb = -20.0;
    double m_freqLow = 4000.0;
    double m_freqHigh = 9000.0;
    double m_reductionDb = -10.0;
    int m_frameSize = 1024;
    double m_sampleRate = 44100.0;

    // Window function
    QVector<double> m_window;

    // Band-pass filter coefficients for sibilant range
    QVector<double> m_bpB, m_bpA;

    // Envelope follower state
    double m_envelope = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;

    /** @brief Design band-pass filter for sibilant frequency range */
    void designBandpass();

    /** @brief Apply band-pass filter */
    QVector<double> applyBandpass(const QVector<double>& input) const;

    /** @brief Compute envelope of signal */
    double computeEnvelope(const QVector<double>& signal);
};
