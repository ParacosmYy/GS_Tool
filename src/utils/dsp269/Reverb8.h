/**
 * @file Reverb8.h
 * @brief 混响器(Schroeder并行梳状滤波与级联全通网络人工房间脉冲响应模拟) — Reverb with Schroeder Parallel Comb and Series All-Pass Network for Artificial Room Impulse Response Simulation
 *
 * 功能: 实现混响器(Reverb)，采用Schroeder并行梳状滤波器(parallel comb filters)
 *       与级联全通网络(series all-pass network)模拟人工房间脉冲响应(artificial room IR)。
 *
 * 协作: FIRFilter7(FIR滤波器) / IIRFilter6(IIR滤波器) / BiquadFilter8(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混响器(Schroeder并行梳状滤波与级联全通网络人工房间脉冲响应模拟)
 */
class Reverb8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numCombFilters = 0;
        int numAllPassFilters = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb8(QObject *parent = nullptr);
    ~Reverb8() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set reverb decay time (RT60) in seconds */
    void setDecayTime(double seconds);

    /** @brief Set wet/dry mix ratio (0..1, 1 = full wet) */
    void setWetDryMix(double mix);

    /** @brief Process a block of samples, returns reverb output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Generate the full impulse response */
    QVector<double> impulseResponse(int length) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

    /** @brief Clear all filter delay lines */
    void reset();

signals:
    void processingCompleted(int blockSize, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_decayTime = 2.0;
    double m_wetDry = 0.5;

    // Schroeder comb filter delay lines
    struct CombFilter {
        QVector<double> buffer;
        int delay = 0;
        int index = 0;
        double feedback = 0.0;
    };

    // All-pass filter delay lines
    struct AllPassFilter {
        QVector<double> buffer;
        int delay = 0;
        int index = 0;
        double feedback = 0.5;
    };

    QVector<CombFilter> m_combs;
    QVector<AllPassFilter> m_allPasses;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize filter delay lines from parameters */
    void initFilters();

    /** @brief Process one sample through a single comb filter */
    double processComb(CombFilter& comb, double input);

    /** @brief Process one sample through a single all-pass filter */
    double processAllPass(AllPassFilter& ap, double input);
};
