/**
 * @file Deesser9.h
 * @brief 去齿音器(心理声学加权齿音检测与多频带动态抑制自然语音处理) — De-esser with Psychoacoustic-weighted Sibilance Detection and Multiband Dynamic Suppression for Natural Speech Processing
 *
 * 功能: 实现去齿音器(De-esser)，采用心理声学加权齿音检测(psychoacoustic-weighted sibilance detection)
 *       与多频带动态抑制(multiband dynamic suppression)实现自然语音处理(natural speech processing)。
 *
 * 协作: Compressor6(动态压缩) / Equalizer8(均衡器) / Limiter5(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音器(心理声学加权齿音检测与多频带动态抑制)
 */
class Deesser9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int sampleRate = 44100;
        double sibilanceRate = 0.0;
        double avgReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser9(QObject *parent = nullptr);
    ~Deesser9() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(int rate);

    /** @brief Set sibilance detection frequency range (Hz) */
    void setSibilanceBand(double lowHz, double highHz);

    /** @brief Set threshold in dB above which sibilance is detected */
    void setThreshold(double thresholdDb);

    /** @brief Set maximum gain reduction in dB */
    void setMaxReduction(double maxReductionDb);

    /** @brief Set attack time in milliseconds */
    void setAttack(double ms);

    /** @brief Set release time in milliseconds */
    void setRelease(double ms);

    /** @brief Process audio buffer, returns de-essed samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get detected sibilance envelope */
    QVector<double> sibilanceEnvelope() const;

    /** @brief Get gain reduction curve */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numSamples, double sibilanceRate, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_lowHz = 4000.0;
    double m_highHz = 9000.0;
    double m_thresholdDb = -20.0;
    double m_maxReductionDb = -12.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;

    QVector<double> m_sibilanceEnv;
    QVector<double> m_gainReduction;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Filter state for bandpass
    double m_x1 = 0.0, m_x2 = 0.0;
    double m_y1 = 0.0, m_y2 = 0.0;
    double m_a0 = 1.0, m_a1 = 0.0, m_a2 = 0.0;
    double m_b0 = 1.0, m_b1 = 0.0, m_b2 = 0.0;

    // Envelope follower state
    double m_envLevel = 0.0;
    double m_gainState = 0.0;

    /** @brief Design bandpass filter for sibilance detection */
    void designBandpass();

    /** @brief Apply bandpass filter to single sample */
    double bandpassSample(double x);

    /** @brief Compute A-weighted psychoacoustic level */
    double aWeightedLevel(double sibilanceLevel) const;
};
