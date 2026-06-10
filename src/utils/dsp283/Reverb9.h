/**
 * @file Reverb9.h
 * @brief 混响(后反射扩散场与早反射抽头延迟的混合房间声学仿真) — Reverb with Late-reflection Diffuse Field and Early-reflection Tapped Delay for Hybrid Room Acoustics Simulation
 *
 * 功能: 实现混响(reverb)，采用后反射扩散场(late-reflection diffuse field)
 *       与早反射抽头延迟(early-reflection tapped delay)实现混合房间声学仿真(hybrid room acoustics simulation)。
 *
 * 协作: IIR9(IIR滤波器) / FIR8(FIR滤波器) / FFT10(FFT频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混响(后反射扩散场与早反射抽头延迟)
 */
class Reverb9 : public QObject {
    Q_OBJECT

public:
    /** @brief Reverb parameters */
    struct ReverbParams {
        double roomSize = 0.7;
        double damping = 0.5;
        double wetLevel = 0.33;
        double dryLevel = 0.4;
        double width = 1.0;
        double preDelay = 0.02;
    };

    /** @brief Processed output */
    struct ReverbOutput {
        QVector<double> audio;
        double peakLevel = 0.0;
        double rmsLevel = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb9(QObject *parent = nullptr);
    ~Reverb9() override;

    void setParams(const ReverbParams& params);
    void setSampleRate(double sr);

    ReverbOutput process(const QVector<double>& input);

    /** @brief Reset internal delay lines */
    void clearBuffers();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int n, double peak, double timeMs);

private:
    ReverbParams m_params;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Early reflection tapped delay line
    QVector<double> m_earlyBuffer;
    int m_earlyPos = 0;

    // Late reflection: 4 parallel comb filters + 4 allpass filters
    struct CombFilter {
        QVector<double> buffer;
        int pos = 0;
        double feedback = 0.0;
        double damp1 = 0.0;
        double damp2 = 0.0;
        double filterStore = 0.0;
    };

    struct AllpassFilter {
        QVector<double> buffer;
        int pos = 0;
        double feedback = 0.0;
    };

    CombFilter m_combs[4];
    AllpassFilter m_allpass[4];

    // Early reflection tap delays (samples) and gains
    QVector<int> m_earlyTaps;
    QVector<double> m_earlyGains;

    /** @brief Initialize filter structures */
    void initFilters();

    /** @brief Process one sample through comb filter */
    double processComb(CombFilter& c, double input);

    /** @brief Process one sample through allpass filter */
    double processAllpass(AllpassFilter& ap, double input);

    /** @brief Generate early reflection taps */
    void generateEarlyTaps();
};
