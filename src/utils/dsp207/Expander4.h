/**
 * @file Expander4.h
 * @brief 动态扩展器(多频段向上压缩+瞬态感知谐波增强) — Dynamic Expander with Multiband Upward Compression and Transient-Aware Harmonic Enhancement
 *
 * 功能: 实现多频段动态扩展器，支持向上压缩、
 *       瞬态检测驱动的谐波增强和自适应增益控制。
 *
 * 协作: Compressor3(压缩器) / GateLimiter5(门限限制) / Equalizer6(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态扩展器(多频段向上压缩+瞬态感知谐波增强)
 */
class Expander4 : public QObject {
    Q_OBJECT

public:
    /** @brief Single band parameters */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -40.0;    // dB
        double ratio = 2.0;
        double attack = 5.0;         // ms
        double release = 50.0;       // ms
        double makeupGain = 0.0;     // dB
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numBands = 0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander4(QObject *parent = nullptr);
    ~Expander4() override;

    void setSampleRate(double sr);
    void setBands(const QVector<BandConfig>& bands);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample through all bands */
    double processSample(double sample);

    /** @brief Detect transient via spectral flux */
    bool detectTransient(const QVector<double>& window) const;

    /** @brief Apply harmonic enhancement to a sample */
    double enhanceHarmonics(double sample, double transientStrength) const;

    /** @brief Get per-band gain envelope (dB) */
    QVector<double> getGainEnvelopes() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    QVector<BandConfig> m_bands;

    /** @brief Per-band state */
    struct BandState {
        double envelope = 0.0;
        double gain = 1.0;
        double prevSample = 0.0;
        QVector<double> firCoeffs;
    };

    QVector<BandState> m_bandStates;
    double m_prevTransient = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute FIR crossover coefficients */
    static QVector<double> designCrossover(double lowFreq, double highFreq,
                                             double sr, int order);

    /** @brief Apply FIR filter to sample (needs history) */
    double applyFIR(double sample, const QVector<double>& coeffs,
                     QVector<double>& history) const;

    /** @brief Compute RMS level in dB */
    static double toDB(double linear);

    /** @brief Convert dB to linear gain */
    static double fromDB(double db);
};
