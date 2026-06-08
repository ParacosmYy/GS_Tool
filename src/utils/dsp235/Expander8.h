/**
 * @file Expander8.h
 * @brief 动态范围扩展器(程序依赖攻击/释放时间+可变拐点软扩展曲线) — Dynamic Range Expander with Program-Dependent Attack/Release and Variable Knee Soft-Expansion Curve
 *
 * 功能: 实现动态范围扩展器(dynamic range expander)，采用程序依赖(program-dependent)
 *       自适应攻击/释放时间(adaptive attack/release)，配合可变拐点(variable knee)
 *       软扩展曲线(soft-expansion curve)，用于音频信号的低电平门控与动态增强。
 *
 * 协作: MultibandCompressor3(多频段压缩) / Limiter5(限幅器) / Gate4(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围扩展器(程序依赖攻击/释放+可变拐点软扩展)
 */
class Expander8 : public QObject {
    Q_OBJECT

public:
    /** @brief Gain envelope point */
    struct EnvelopePoint {
        double sample = 0.0;
        double gainDb = 0.0;
        double inputDb = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double avgGainReduction = 0.0;
        double peakReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander8(QObject *parent = nullptr);
    ~Expander8() override;

    /** @brief Set expansion ratio (1:1 = no expansion, >1 = expand) */
    void setRatio(double ratio);

    /** @brief Set threshold in dB (signals below this are expanded) */
    void setThreshold(double thresholdDb);

    /** @brief Set attack time in ms */
    void setAttack(double ms);

    /** @brief Set release time in ms */
    void setRelease(double ms);

    /** @brief Set knee width in dB (0 = hard knee) */
    void setKnee(double kneeDb);

    /** @brief Set sample rate */
    void setSampleRate(double sr);

    /** @brief Process samples, returns gain-adjusted output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get gain envelope history */
    QVector<EnvelopePoint> envelope() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double avgReduction, double timeMs);

private:
    double m_ratio = 2.0;
    double m_thresholdDb = -30.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    double m_kneeDb = 6.0;
    double m_sampleRate = 44100.0;

    double m_envelopeDb = 0.0;      // Current gain envelope (dB)
    double m_gainReductionSum = 0.0;
    int m_grCount = 0;

    QVector<EnvelopePoint> m_envelope;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute expansion gain from input level */
    double computeGain(double inputDb) const;

    /** @brief Compute program-dependent coefficient */
    double envelopeCoeff(double targetDb, double currentDb) const;

    /** @brief Convert amplitude to dB */
    double ampToDb(double amp) const;

    /** @brief Convert dB to amplitude */
    double dbToAmp(double db) const;
};
