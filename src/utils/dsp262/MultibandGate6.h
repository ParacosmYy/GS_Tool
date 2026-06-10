/**
 * @file MultibandGate6.h
 * @brief 多频段门控(交叉滤波器组独立逐带阈值释放包络整形) — Multiband Gate with Crossover Filter Bank and Independent Per-band Threshold with Release Envelope Shaping
 *
 * 功能: 实现多频段门控(multiband gate)，采用交叉滤波器组(crossover filter bank)
 *       进行独立逐带阈值(independent per-band threshold)检测，配合释放包络整形
 *       (release envelope shaping)实现平滑频段增益控制。
 *
 * 协作: MultibandCompressor6(多频段压缩) / DynamicEQ6(动态EQ) / NoiseGate6(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控(交叉滤波器组独立逐带阈值释放包络整形)
 */
class MultibandGate6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Per-band gate parameters */
    struct BandConfig {
        double threshold = -40.0;   // dB
        double attack = 1.0;        // ms
        double release = 50.0;      // ms
        double ratio = 10.0;        // gate ratio
        double range = -80.0;       // max attenuation dB
    };

    explicit MultibandGate6(int numBands = 4, QObject *parent = nullptr);
    ~MultibandGate6() override;

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Set crossover frequencies between bands */
    void setCrossoverFrequencies(const QVector<double>& freqs);

    /** @brief Set gate config for a specific band */
    void setBandConfig(int band, const BandConfig& config);

    /** @brief Process a block of interleaved stereo samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current per-band gain levels (dB) */
    QVector<double> bandGains() const;

    /** @brief Get per-band RMS levels */
    QVector<double> bandLevels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, int numBands, double timeMs);

private:
    int m_numBands;
    double m_sampleRate = 44100.0;
    int m_blockSize = 0;

    QVector<BandConfig> m_configs;
    QVector<double> m_crossoverFreqs;

    // Filter state per band (Linkwitz-Riley 4th-order crossover)
    struct FilterState {
        double x1 = 0.0, x2 = 0.0, x3 = 0.0, x4 = 0.0;
        double y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
    };

    // Per-band envelope state
    struct BandState {
        QVector<FilterState> lpState;    // LP filter states
        QVector<FilterState> hpState;    // HP filter states
        double envelope = 0.0;           // current envelope (linear)
        double gain = 1.0;               // current gain (linear)
        double levelDb = -120.0;         // current level in dB
    };

    QVector<BandState> m_bandStates;
    QVector<double> m_bandGains;
    QVector<double> m_bandLevels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply 2nd-order Linkwitz-Riley lowpass */
    double processLP(FilterState& s, double input, double freq);

    /** @brief Apply 2nd-order Linkwitz-Riley highpass */
    double processHP(FilterState& s, double input, double freq);

    /** @brief Split signal into frequency bands */
    QVector<QVector<double>> splitBands(const QVector<double>& input);

    /** @brief Compute gain from envelope and band config */
    double computeGateGain(double envelopeLin, const BandConfig& cfg) const;

    /** @brief Convert dB to linear */
    static double dbToLinear(double db);

    /** @brief Convert linear to dB */
    static double linearToDb(double lin);
};
