/**
 * @file Compressor6.h
 * @brief 动态范围压缩器(软拐点特性曲线+RMS/峰值双检测模式) — Dynamic Range Compressor with Soft-Knee Characteristic Curve and RMS/Peak Dual Detection Mode
 *
 * 功能: 实现动态范围压缩器(dynamic range compressor)，支持软拐点(soft-knee)
 *       特性曲线实现平滑拐点过渡，提供RMS和峰值(peak)双检测模式(dual detection)
 *       计算信号电平，带攻击/释放包络跟随器。
 *
 * 协作: AdaptiveFilter7(自适应滤波) / EnvelopeDetector4(包络检测) / Limiter3(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器(软拐点特性曲线+RMS/峰值双检测模式)
 */
class Compressor6 : public QObject {
    Q_OBJECT

public:
    /** @brief Detection mode for signal level */
    enum DetectionMode {
        RMS = 0,
        Peak = 1
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double inputPeakDb = 0.0;
        double outputPeakDb = 0.0;
        double avgGainReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor6(QObject *parent = nullptr);
    ~Compressor6() override;

    /** @brief Set threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Set compression ratio (e.g. 4.0 means 4:1) */
    void setRatio(double ratio);

    /** @brief Set soft-knee width in dB (0 = hard knee) */
    void setKneeWidth(double widthDb);

    /** @brief Set attack time in milliseconds */
    void setAttackTime(double ms);

    /** @brief Set release time in milliseconds */
    void setReleaseTime(double ms);

    /** @brief Set detection mode */
    void setDetectionMode(DetectionMode mode);

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Process input samples and return compressed output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get gain reduction envelope in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void compressionCompleted(int numSamples, double avgGainReductionDb, double timeMs);

private:
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_kneeWidthDb = 6.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    DetectionMode m_mode = RMS;
    double m_sampleRate = 44100.0;

    double m_envelope = 0.0;
    QVector<double> m_gainReduction;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute gain reduction for a given input level in dB */
    double computeGainReduction(double inputDb) const;

    /** @brief Amplitude to dB */
    static double toDb(double amp);

    /** @brief dB to amplitude */
    static double fromDb(double db);
};
