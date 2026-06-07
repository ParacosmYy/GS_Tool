/**
 * @file Reverb2.h
 * @brief 算法混响(Schroeder级联4梳状+2全通+预延迟+早期反射) — Algorithmic Reverb via Schroeder Series with Pre-delay and Early Reflections
 *
 * 功能: 实现Schroeder混响算法，支持4个梳状滤波器(Comb)级联、
 *       2个全通滤波器(Allpass)、可调预延迟和早期反射仿真。
 *
 * 协作: FirFilter4(FIR滤波器) / IirFilter5(IIR滤波器) / DelayLine3(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 算法混响器(Schroeder级联+预延迟+早期反射)
 */
class Reverb2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int combCount = 0;
        int allpassCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb2(QObject *parent = nullptr);
    ~Reverb2() override;

    void setSampleRate(double sr);
    void setPreDelay(double delayMs);
    void setRoomSize(double size);
    void setDamping(double damping);
    void setWetDryMix(double wet);
    void setEarlyReflectionGain(double gain);

    /** @brief Process a single sample through the reverb */
    double processSample(double input);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Reset all delay lines and state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_preDelayMs = 20.0;
    double m_roomSize = 0.7;
    double m_damping = 0.5;
    double m_wetMix = 0.3;
    double m_earlyGain = 0.4;

    // Pre-delay buffer
    QVector<double> m_preBuf;
    int m_prePos = 0;

    // Comb filter state (4 comb filters)
    struct CombFilter {
        QVector<double> buffer;
        int pos = 0;
        double feedback = 0.0;
        double filterStore = 0.0;
        double damp1 = 0.0;
        double damp2 = 0.0;
    };
    QVector<CombFilter> m_combs;

    // Allpass filter state (2 allpass filters)
    struct AllpassFilter {
        QVector<double> buffer;
        int pos = 0;
        double feedback = 0.0;
    };
    QVector<AllpassFilter> m_allpasses;

    // Early reflection taps (delay samples)
    QVector<QPair<int, double>> m_earlyTaps;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize comb and allpass filter delay lines */
    void initFilters();

    /** @brief Process through a single comb filter with LP damping */
    double processComb(CombFilter& c, double input);

    /** @brief Process through a single allpass filter */
    double processAllpass(AllpassFilter& ap, double input);

    /** @brief Generate early reflection pattern */
    void generateEarlyReflections();
};
