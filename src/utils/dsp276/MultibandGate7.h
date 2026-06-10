/**
 * @file MultibandGate7.h
 * @brief 多频段门限(逐频段动态压缩与瞬态检测的多通道动态控制) — Multiband Gate with Dynamic Compression per Band and Transient Detection for Multichannel Dynamic Control
 *
 * 功能: 实现多频段门限(Multiband gate)，采用逐频段动态压缩(dynamic compression per band)
 *       与瞬态检测(transient detection)实现多通道动态控制(multichannel dynamic control)。
 *
 * 协作: BiquadFilter10(双二阶滤波器) / EnvelopeDetector6(包络检测) / DynamicCompressor8(动态压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门限(逐频段动态压缩与瞬态检测)
 */
class MultibandGate7 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band configuration */
    struct BandConfig {
        double lowFreq = 0.0;       // Lower crossover frequency
        double highFreq = 0.0;      // Upper crossover frequency
        double threshold = -30.0;   // Gate threshold in dB
        double ratio = 4.0;         // Compression ratio below threshold
        double attack = 1.0;        // Attack time in ms
        double release = 50.0;      // Release time in ms
        double makeupGain = 0.0;    // Makeup gain in dB
        bool enabled = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
        double peakReductionDb = 0.0;
    };

    explicit MultibandGate7(QObject *parent = nullptr);
    ~MultibandGate7() override;

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set number of frequency bands */
    void setNumBands(int n);

    /** @brief Configure a specific band */
    void setBandConfig(int band, const BandConfig& config);

    /** @brief Process a block of interleaved multichannel samples */
    QVector<double> process(const QVector<double>& input, int channels);

    /** @brief Get per-band gain (dB) after last process call */
    QVector<double> bandGains() const;

    /** @brief Get transient flags per band from last process call */
    QVector<bool> transients() const;

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numBands, int blockSize, double peakReduction, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    int m_channels = 1;

    QVector<BandConfig> m_bandConfigs;

    // Per-band state
    QVector<double> m_envelope;       // Current envelope level per band
    QVector<double> m_gain;           // Current gain per band
    QVector<bool> m_transient;        // Transient detected per band
    QVector<double> m_prevEnvelope;   // Previous envelope for transient detection

    // Linkwitz-Riley crossover filters (2nd order per side = 4th order total)
    struct LRFilter {
        // State for each 2nd-order section (2 sections for LR4)
        double x1[2] = {};
        double x2[2] = {};
        double y1[2] = {};
        double y2[2] = {};
        double b0[2] = {}, b1[2] = {}, b2[2] = {};
        double a1[2] = {}, a2[2] = {};
    };

    QVector<LRFilter> m_lowpass;      // Lowpass filter per crossover
    QVector<LRFilter> m_highpass;     // Highpass filter per crossover

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design Linkwitz-Riley crossover for given frequency */
    void designCrossover(LRFilter& lp, LRFilter& hp, double freq);

    /** @brief Process one sample through a biquad section */
    double processBiquad(double in, LRFilter& f, int section);

    /** @brief Compute gain from envelope and band config */
    double computeGateGain(double envelope, const BandConfig& cfg) const;

    /** @brief Detect transient in envelope change */
    bool detectTransient(double current, double previous, double threshold) const;

    /** @brief Split signal into bands using crossover filters */
    QVector<QVector<double>> splitBands(const QVector<double>& input, int channels);

    /** @brief Initialize filter coefficients */
    void initFilters();
};
