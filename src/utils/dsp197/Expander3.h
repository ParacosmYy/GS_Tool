/**
 * @file Expander3.h
 * @brief 下行扩展器(程序依赖attack/release+立体声链路) — Downward Expander with Program-Dependent Attack/Release and Stereo Link for Coherent Imaging
 *
 * 功能: 实现下行扩展器，支持程序依赖的attack/release时间、
 *       立体声链路和一致性成像。
 *
 * 协作: Compressor7(压缩器) / Limiter5(限制器) / NoiseGate4(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 下行扩展器(程序依赖包络+立体声链路)
 */
class Expander3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int sampleRate = 44100;
        double avgGainReduction = 0.0;
        double peakGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander3(QObject *parent = nullptr);
    ~Expander3() override;

    void setThreshold(double dB);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setRange(double dB);
    void setSampleRate(int sr);
    void setStereoLinkEnabled(bool enabled);
    void setKneeWidth(double dB);

    /** @brief Process interleaved stereo samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process mono samples */
    QVector<double> processMono(const QVector<double>& input);

    /** @brief Compute gain for a given input level */
    double computeGain(double inputLeveldB) const;

    /** @brief Get current envelope level (dB) */
    double envelopeLevel() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double peakReduction, double timeMs);

private:
    double m_threshold = -40.0;   // dB
    double m_ratio = 2.0;        // expansion ratio
    double m_attack = 10.0;      // ms
    double m_release = 100.0;    // ms
    double m_range = -60.0;      // max expansion dB
    int m_sampleRate = 44100;
    bool m_stereoLink = true;
    double m_kneeWidth = 6.0;    // dB

    // Envelope state
    double m_envelope = 0.0;
    double m_envAttackCoeff = 0.0;
    double m_envReleaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update envelope coefficients from timing params */
    void updateCoefficients();

    /** @brief Detect level in dB from sample */
    static double sampleToDB(double sample);

    /** @brief Convert dB to linear gain */
    static double dbToLinear(double dB);

    /** @brief Convert linear to dB */
    static double linearToDB(double linear);

    /** @brief Program-dependent attack from transient */
    double programAttack(double inputdB) const;

    /** @brief Program-dependent release from sustain */
    double programRelease(double inputdB) const;
};
