/**
 * @file Compressor4.h
 * @brief 动态范围压缩器(前馈增益计算+软拐点对数特性曲线) — Dynamic Range Compressor with Feed-Forward Gain Computation and Soft-Knee Logarithmic Characteristic Curve
 *
 * 功能: 实现动态范围压缩器，支持前馈增益检测、
 *       软拐点对数特性曲线、可调攻击/释放时间包络跟随。
 *
 * 协作: EnvelopeDetector3(包络检测) / AdaptiveFilter5(自适应滤波) / Equalizer2(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器(前馈+软拐点)
 */
class Compressor4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int sampleRate = 0;
        double thresholdDb = 0.0;
        double ratio = 0.0;
        double kneeWidthDb = 0.0;
        double gainReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor4(QObject *parent = nullptr);
    ~Compressor4() override;

    /** @brief Set compressor parameters */
    void setParameters(double thresholdDb, double ratio, double kneeWidthDb,
                       double attackMs, double releaseMs, int sampleRate = 44100,
                       double makeupGainDb = 0.0);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process a single sample */
    double processOne(double sample);

    /** @brief Compute gain reduction for a given level in dB */
    double computeGainReduction(double inputLevelDb) const;

    /** @brief Get current envelope level */
    double envelopeLevel() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double gainReduction, double timeMs);

private:
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_kneeWidthDb = 10.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    int m_sampleRate = 44100;
    double m_makeupGainDb = 0.0;

    // Envelope follower state
    double m_envelope = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update attack/release coefficients */
    void updateCoefficients();

    /** @brief Soft-knee transfer function */
    double transferFunction(double inputDb) const;

    /** @brief Convert amplitude to dB */
    static double ampToDb(double amp);

    /** @brief Convert dB to amplitude */
    static double dbToAmp(double db);
};
