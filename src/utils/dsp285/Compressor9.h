/**
 * @file Compressor9.h
 * @brief 多频段动态压缩器(侧链与可变交叉淡入比) — Multiband Compressor with Sidechain and Variable Crossfade Ratio for Transparent Frequency-dependent Dynamic Processing
 *
 * 功能: 实现多频段动态压缩器(multiband compressor)，采用侧链(sidechain)
 *       与可变交叉淡入比(variable crossfade ratio)实现透明频段相关动态处理(transparent frequency-dependent dynamic processing)。
 *
 * 协作: Equalizer8(均衡器) / Limiter7(限幅器) / Expander6(扩展器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段动态压缩器(侧链与可变交叉淡入比)
 */
class Compressor9 : public QObject {
    Q_OBJECT

public:
    /** @brief Single band compressor parameters */
    struct BandParams {
        double lowFreq = 0.0;       // Band lower crossover frequency (Hz)
        double highFreq = 0.0;      // Band upper crossover frequency (Hz)
        double threshold = -20.0;   // Threshold in dB
        double ratio = 4.0;         // Compression ratio
        double attack = 10.0;       // Attack time in ms
        double release = 100.0;     // Release time in ms
        double knee = 6.0;          // Soft knee width in dB
        double makeupGain = 0.0;    // Makeup gain in dB
        double mix = 1.0;           // Dry/wet mix (0..1)
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        QVector<double> gainReduction;   // Per-sample gain reduction in dB
        double peakReduction = 0.0;
        double outputPeak = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor9(int numBands = 4, QObject *parent = nullptr);
    ~Compressor9() override;

    void setSampleRate(double sr);
    void setBandParams(int band, const BandParams& params);
    void setCrossfadeRatio(double ratio);
    void setSidechainSource(const QVector<double>& sc);

    /** @brief Process audio through multiband compressor */
    ProcessResult process(const QVector<double>& input);

    /** @brief Compute gain reduction for single band given level in dB */
    double computeGainReduction(double inputDb, const BandParams& band) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int n, int bands, double peakRed, double timeMs);

private:
    int m_numBands;
    double m_sampleRate = 44100.0;
    double m_crossfade = 0.5;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<BandParams> m_bands;
    QVector<QVector<double>> m_crossoverCoeffs;  // LP/HP coefficients per crossover
    QVector<double> m_sidechain;

    // Envelope state per band
    QVector<double> m_envState;

    /** @brief Design crossover Linkwitz-Riley filter coefficients */
    void designCrossovers();

    /** @brief Apply crossover filter bank, return per-band signals */
    QVector<QVector<double>> applyCrossoverBank(const QVector<double>& input);

    /** @brief Apply single-band compression envelope */
    double processEnvelope(int band, double input, double samplePeriod);

    /** @brief dB <-> linear conversions */
    static double toDb(double linear);
    static double fromDb(double db);

    /** @brief Second-order LPF state per crossover */
    struct FilterState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };
    QVector<QVector<FilterState>> m_filterStates;
};
