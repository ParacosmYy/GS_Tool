/**
 * @file MultibandCompressor6.h
 * @brief 多频段压缩器(FIR交叉分频+逐频段前瞻限制+频段间增益共享) — Multiband Compressor with FIR Crossover and Lookahead Per-band Limiting with Inter-band Gain Sharing
 *
 * 功能: 实现多频段动态范围压缩器，使用FIR交叉滤波器(crossover)分频，
 *       每频段独立前瞻限制(lookahead limiting)，并通过频段间增益共享(inter-band gain sharing)减少失真。
 *
 * 协作: IIRFilter3(IIR滤波器) / FIRFilter2(FIR滤波器) / DynamicProcessor4(动态处理)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(FIR交叉分频+前瞻限制+增益共享)
 */
class MultibandCompressor6 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor configuration */
    struct BandConfig {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 5.0;        // ms
        double release = 50.0;      // ms
        double makeupGain = 0.0;    // dB
        bool bypass = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int numTaps = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor6(QObject *parent = nullptr);
    ~MultibandCompressor6() override;

    /** @brief Configure: number of bands, crossover freqs, FIR taps */
    bool configure(int numBands, const QVector<double>& crossoverFreqs,
                   int firTaps, double sampleRate);

    /** @brief Set compressor parameters for a specific band */
    void setBandConfig(int band, const BandConfig& config);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get per-band gain reduction (dB) after last process call */
    QVector<double> gainReduction() const;

    /** @brief Reset filter states and envelope followers */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);
    void gainReductionChanged(const QVector<double>& reductionDb);

private:
    int m_numBands = 4;
    int m_firTaps = 64;
    double m_sampleRate = 44100.0;
    int m_lookaheadSamples = 64;

    QVector<BandConfig> m_bandConfigs;
    QVector<QVector<double>> m_crossoverCoeffs;  // FIR coefficients per crossover
    QVector<QVector<double>> m_filterStates;     // FIR delay lines
    QVector<double> m_envelopeState;             // envelope follower per band
    QVector<double> m_gainReduction;
    QVector<double> m_lookaheadBuffer;           // lookahead delay buffer
    int m_lookaheadPos = 0;

    // Gain sharing state
    double m_sharedGain = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design FIR crossover filter using windowed sinc */
    QVector<double> designCrossoverFIR(double cutoffFreq, int taps) const;

    /** @brief Apply FIR filter to sample with given delay line */
    double applyFIR(const QVector<double>& coeffs, QVector<double>& state,
                    double sample) const;

    /** @brief Compute gain reduction for a band given level */
    double computeGainReduction(double levelDb, const BandConfig& cfg) const;

    /** @brief Envelope follower with attack/release */
    double envelopeFollow(double current, double previous,
                          double attackCoeff, double releaseCoeff) const;

    /** @brief Inter-band gain sharing to reduce spectral distortion */
    double sharedGainAdjustment(const QVector<double>& reductions) const;
};
