/**
 * @file Expander12.h
 * @brief 扩展器(程序依赖包络跟踪频谱泄漏控制透明动态缩减) — Expander with Program-dependent Attack/Release and Spectral Bleed Control for Transparent Dynamic Reduction
 *
 * 功能: 实现扩展器(Expander)，采用程序依赖包络跟踪(program-dependent envelope
 *       tracking)和频谱泄漏控制(spectral bleed control)实现透明动态缩减
 *       (transparent dynamic reduction)。
 *
 * 协作: Compressor10(压缩器) / Limiter8(限制器) / Gate7(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 扩展器(程序依赖包络跟踪频谱泄漏控制透明动态缩减)
 */
class Expander12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        double sampleRate = 44100.0;
        int blockSize = 0;
        double avgGainReduction = 0.0;
        double peakReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander12(QObject *parent = nullptr);
    ~Expander12() override;

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Set expander parameters */
    void setParameters(double threshold, double ratio, double range,
                       double attack, double release, double knee);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gainReductionComputed(double avgReduction, double peakReduction, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_threshold = -20.0;     // dB
    double m_ratio = 2.0;
    double m_range = -60.0;         // max gain reduction dB
    double m_attack = 10.0;         // ms
    double m_release = 100.0;       // ms
    double m_knee = 6.0;            // dB

    // Envelope state
    double m_envelope = 0.0;
    double m_gainDb = 0.0;

    // Spectral bleed control
    QVector<double> m_prevSpectrum;
    int m_fftSize = 256;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;
    int m_reductionCount = 0;

    /** @brief Convert time in ms to coefficient */
    double timeToCoeff(double timeMs) const;

    /** @brief Compute gain from envelope with soft knee */
    double computeGain(double inputDb) const;

    /** @brief Estimate spectral bleed and apply correction */
    double spectralBleedCorrection(const QVector<double>& block) const;
};
