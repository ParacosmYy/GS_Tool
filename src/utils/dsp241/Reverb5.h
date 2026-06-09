/**
 * @file Reverb5.h
 * @brief 施罗德混响(并联梳状滤波器+全通反馈环路) — Schroeder Reverb with Parallel Comb Filter Series and All-Pass Feedback Loop with Configurable Decay Time
 *
 * 功能: 实现施罗德混响算法(Schroeder reverb)，采用并联梳状滤波器系列(parallel comb
 *       filter series)产生早期反射密度，通过全通反馈环路(all-pass feedback loop)扩散
 *       回声尾部，支持可配置衰减时间(decay time)调节混响持续时长。
 *
 * 协作: IIRFilter3(IIR滤波器) / FIRFilter2(FIR滤波器) / SignalGenerator6(信号发生器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 施罗德混响(并联梳状滤波器+全通反馈环路)
 */
class Reverb5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numChannels = 0;
        int sampleRate = 0;
        double decayTimeMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb5(QObject *parent = nullptr);
    ~Reverb5() override;

    /** @brief Configure sample rate (Hz) */
    void setSampleRate(int rate);

    /** @brief Set reverb decay time T60 (ms) */
    void setDecayTime(double ms);

    /** @brief Set wet/dry mix ratio [0..1] */
    void setWetDryMix(double mix);

    /** @brief Process entire buffer */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample */
    double processOne(double sample);

    /** @brief Reset internal delay lines */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_decayTimeMs = 1500.0;
    double m_wetDry = 0.5;

    // Comb filter delay lines and feedback gains
    static constexpr int kNumCombs = 4;
    static constexpr int kNumAllPass = 2;

    QVector<QVector<double>> m_combBuffers;
    QVector<double> m_combFeedback;
    QVector<int> m_combIndex;
    QVector<int> m_combDelay;

    QVector<QVector<double>> m_apBuffers;
    QVector<double> m_apFeedback;
    QVector<int> m_apIndex;
    QVector<int> m_apDelay;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize comb and all-pass delay line parameters */
    void initFilters();

    /** @brief Read from circular delay buffer */
    static double readDelay(const QVector<double>& buf, int idx);

    /** @brief Write to circular delay buffer and advance */
    static void writeDelay(QVector<double>& buf, int& idx, double val);
};
