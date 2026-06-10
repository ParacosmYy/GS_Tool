/**
 * @file Compressor8.h
 * @brief 动态范围压缩器(侧链检测与自动增益补偿软拐点压缩曲线) — Compressor with Sidechain Detection and Auto-Gain Make-Up with Soft-Knee Compression Curve for Dynamic Range Control
 *
 * 功能: 实现动态范围压缩器(compressor)，采用侧链检测(sidechain detection)
 *       与自动增益补偿(auto-gain make-up)及软拐点压缩曲线(soft-knee compression curve)实现动态范围控制(dynamic range control)。
 *
 * 协作: EnvelopeDetector5(包络检测) / DeEsser3(齿音消除) / Limiter6(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器(侧链检测与自动增益补偿软拐点压缩曲线)
 */
class Compressor8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        double peakGainReduction = 0.0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor8(QObject *parent = nullptr);
    ~Compressor8() override;

    /** @brief Set threshold in dB (level above which compression starts) */
    void setThreshold(double thresholdDb);

    /** @brief Set compression ratio (e.g., 4.0 means 4:1) */
    void setRatio(double ratio);

    /** @brief Set knee width in dB (0 = hard knee, >0 = soft knee) */
    void setKneeWidth(double kneeDb);

    /** @brief Set attack time in milliseconds */
    void setAttack(double ms);

    /** @brief Set release time in milliseconds */
    void setRelease(double ms);

    /** @brief Set make-up gain in dB */
    void setMakeUpGain(double gainDb);

    /** @brief Process input with optional sidechain signal */
    QVector<double> process(const QVector<double>& input,
                            const QVector<double>& sidechain = {});

    /** @brief Get gain reduction envelope (dB) from last process call */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int samples, double peakReduction, double timeMs);

private:
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_kneeDb = 6.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    double m_makeUpDb = 0.0;

    double m_envelope = 0.0;        // current gain envelope (linear)
    QVector<double> m_gainReduction; // dB reduction per sample

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute gain reduction for a given input level (dB) with soft knee */
    double computeGainReduction(double inputDb) const;

    /** @brief Smooth envelope with attack/release ballistics */
    double smoothEnvelope(double target, double sampleRate);
};
