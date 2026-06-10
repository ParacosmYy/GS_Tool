/**
 * @file Expander13.h
 * @brief 扩展器(向下并行压缩混合与瞬态保持透明动态范围增强) — Expander with Downward Parallel Compression Blend and Transient Preservation for Transparent Dynamic Range Enhancement
 *
 * 功能: 实现扩展器(expander)，采用向下并行压缩混合(downward parallel compression blend)
 *       与瞬态保持(transient preservation)实现透明动态范围增强(transparent dynamic range enhancement)。
 *
 * 协作: Compressor15(压缩器) / Limiter12(限幅器) / GateFilter11(门滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 扩展器(向下并行压缩混合与瞬态保持透明动态范围增强)
 */
class Expander13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double avgGainReduction = 0.0;
        double peakLevel = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander13(QObject *parent = nullptr);
    ~Expander13() override;

    /** @brief Set threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Set expansion ratio (1:1 = no expansion) */
    void setRatio(double ratio);

    /** @brief Set attack time in ms */
    void setAttack(double attackMs);

    /** @brief Set release time in ms */
    void setRelease(double releaseMs);

    /** @brief Set parallel blend amount (0=dry, 1=full expanded) */
    void setBlend(double blend);

    /** @brief Process a block of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current gain reduction in dB */
    double gainReduction() const;

    /** @brief Reset internal envelope state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numSamples, double gainReduction, double timeMs);

private:
    double m_thresholdDb = -40.0;
    double m_ratio = 2.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    double m_blend = 0.8;

    // Envelope follower state
    double m_envelope = 0.0;
    double m_gainReduction = 0.0;

    // Transient preservation: previous sample and transient detector state
    double m_prevInput = 0.0;
    double m_transientCoeff = 0.0;
    double m_prevTransient = 0.0;

    // Sample rate for coefficient computation
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute gain for a given input level in dB */
    double computeGain(double inputDb) const;

    /** @brief Detect transient via first-order difference */
    double detectTransient(double input);

    /** @brief Smooth envelope with separate attack/release */
    double smoothEnvelope(double inputLevel);
};
