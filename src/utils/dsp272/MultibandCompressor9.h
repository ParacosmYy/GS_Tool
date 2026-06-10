/**
 * @file MultibandCompressor9.h
 * @brief 多频段压缩器(Linkwitz-Riley交叉与逐频段侧链独立比率/阈值控制) — Multiband Compressor with Linkwitz-Riley Crossover and Per-Band Sidechain with Independent Ratio/Threshold Control
 *
 * 功能: 实现多频段压缩器(multiband compressor)，采用Linkwitz-Riley交叉滤波器(Linkwitz-Riley crossover)
 *       与逐频段侧链(per-band sidechain)实现独立比率/阈值控制(independent ratio/threshold control)。
 *
 * 协作: ButterworthFilter8(巴特沃斯) / PeakLimiter7(峰值限制) / DynamicExpander6(动态扩展)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(Linkwitz-Riley交叉与逐频段侧链独立比率/阈值控制)
 */
class MultibandCompressor9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 10.0;       // ms
        double release = 100.0;     // ms
        double kneeWidth = 6.0;     // dB
        double makeupGain = 0.0;    // dB
    };

    explicit MultibandCompressor9(QObject *parent = nullptr);
    ~MultibandCompressor9() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double sr);

    /** @brief Set crossover frequencies (N-1 freqs for N bands) */
    void setCrossoverFreqs(const QVector<double>& freqs);

    /** @brief Set parameters for a specific band */
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a block of samples, returns compressed output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numBands, int blockSize, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    QVector<double> m_crossoverFreqs;
    QVector<BandParams> m_bandParams;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Biquad filter state for Linkwitz-Riley crossover */
    struct BiquadState {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };

    /** @brief Per-band envelope follower state */
    struct BandState {
        QVector<BiquadState> lpState;  // 2nd-order LP per crossover stage
        QVector<BiquadState> hpState;  // 2nd-order HP per crossover stage
        double envelope = 0.0;
        double gainLin = 1.0;
        double gainReductionDb = 0.0;
    };

    QVector<BandState> m_bandStates;
    QVector<double> m_gainReduction;

    /** @brief Initialize Linkwitz-Riley 4th-order crossover filters */
    void initCrossover();

    /** @brief Process one biquad filter sample */
    double processBiquad(double sample, BiquadState& state) const;

    /** @brief Compute soft-knee gain reduction */
    double computeGainReduction(double inputDb, const BandParams& params) const;

    /** @brief Compute envelope follower for one sample */
    double followEnvelope(double sample, double attackCoeff,
                          double releaseCoeff, double& envelope) const;
};
