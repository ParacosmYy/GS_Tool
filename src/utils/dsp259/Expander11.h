/**
 * @file Expander11.h
 * @brief 下行扩展器(程序依赖释放+智能迟滞门控自然瞬态保留) — Downward Expander with Program-dependent Release and Smart Gate with Hysteresis for Natural Transient Preservation
 *
 * 功能: 实现下行扩展器(downward expander)，采用程序依赖释放时间
 *       (program-dependent release)和带迟滞(hysteresis)的智能门控
 *       (smart gate)，实现自然瞬态保留(natural transient preservation)。
 *
 * 协作: Compressor10(动态压缩器) / Limiter7(限幅器) / Gate8(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 下行扩展器(程序依赖释放+智能迟滞门控自然瞬态保留)
 */
class Expander11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        double peakGainReduction = 0.0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander11(QObject *parent = nullptr);
    ~Expander11() override;

    /** @brief Set threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Set expansion ratio (1:ratio below threshold) */
    void setRatio(double ratio);

    /** @brief Set attack time in milliseconds */
    void setAttack(double attackMs);

    /** @brief Set base release time in milliseconds */
    void setRelease(double releaseMs);

    /** @brief Set hysteresis offset in dB for smart gate */
    void setHysteresis(double hysteresisDb);

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Process a single sample */
    double processSample(double sample);

    /** @brief Process a frame of samples in-place */
    void processFrame(QVector<double>& frame);

    /** @brief Get current gain in dB */
    double currentGainDb() const;

    /** @brief Get current envelope level in dB */
    double currentEnvelopeDb() const;

    /** @brief Check if gate is currently open */
    bool isGateOpen() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int size, double gainReduction, double timeMs);

private:
    double m_thresholdDb = -40.0;
    double m_ratio = 4.0;
    double m_attackMs = 0.1;
    double m_releaseMs = 50.0;
    double m_hysteresisDb = 6.0;
    double m_sampleRate = 44100.0;

    double m_gainLinear = 1.0;
    double m_gainDb = 0.0;
    double m_envelopeDb = -120.0;
    bool m_gateOpen = false;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_gainReductionSum = 0.0;

    /** @brief Convert dB to linear */
    double dbToLinear(double db) const;

    /** @brief Convert linear to dB */
    double linearToDb(double linear) const;

    /** @brief Compute program-dependent release time */
    double programRelease(double overshootDb) const;

    /** @brief Smooth gain with attack/release ballistics */
    double smoothGain(double targetDb, double dt);
};
