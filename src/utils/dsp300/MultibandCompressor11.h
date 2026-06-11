/**
 * @file MultibandCompressor11.h
 * @brief 多频段压缩器(线性相位交叉与逐频段侧链滤波实现相位相干多频率动态处理) — Multiband Compressor with Linear-Phase Crossover and Per-Band Sidechain Filter for Phase-Coherent Multi-Frequency Dynamic Processing
 *
 * 功能: 实现多频段压缩器(multiband compressor)，采用线性相位交叉(linear-phase crossover)
 *       与逐频段侧链滤波(per-band sidechain filter)实现相位相干多频率动态处理(phase-coherent multi-frequency dynamic processing)。
 *
 * 协作: DynamicProcessor(动态处理器) / FIRFilter(FIR滤波器) / SpectrumAnalyzer(频谱分析器)
 */
#pragma once

#include <QObject>
#include <QVector>

class MultibandCompressor11 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 10.0;       // ms
        double release = 100.0;     // ms
        double knee = 6.0;          // dB (soft knee width)
        double gain = 0.0;          // makeup gain dB
        double sidechainFreq = 0.0; // sidechain filter center Hz, 0 = bypass
    };

    /** @brief Per-band runtime state */
    struct BandState {
        double envelope = 0.0;
        double gainReduction = 0.0;  // dB
        double rmsLevel = -120.0;    // dB
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        QVector<double> gainReduction;  // per-band GR in dB
        double peakOut = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int numBands = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor11(QObject *parent = nullptr);
    ~MultibandCompressor11() override;

    void setSampleRate(double rate);
    void setNumBands(int bands);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a block of samples through the multiband compressor */
    ProcessResult process(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    const QVector<BandState>& bandStates() const { return m_bandStates; }
    void resetStatistics();

signals:
    void processDone(int frames, double peakReduction, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    QVector<double> m_crossoverFreqs;
    QVector<BandParams> m_bandParams;
    QVector<BandState> m_bandStates;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Linear-phase FIR filter delay lines per band
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_delayPos;
    int m_filterOrder = 64;

    /** @brief Design linear-phase crossover FIR for given cutoff */
    QVector<double> designLPFIR(double cutoffHz, int order) const;

    /** @brief Apply FIR filter to sample with delay line */
    double applyFIR(double sample, int bandIdx, const QVector<double>& coeffs);

    /** @brief Compute gain reduction from envelope */
    double computeGainReduction(double inputDB, const BandParams& params) const;

    /** @brief Apply sidechain filter emphasis to band signal */
    double sidechainFilter(double sample, int bandIdx);

    /** @brief Update envelope follower */
    double updateEnvelope(double input, double attackCoeff, double releaseCoeff, double prevEnvelope) const;
};
