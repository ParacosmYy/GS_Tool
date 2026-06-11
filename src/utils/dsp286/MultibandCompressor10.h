/**
 * @file MultibandCompressor10.h
 * @brief 多频段压缩器(动态频段分割与每频段自动补偿增益) — Multiband Compressor with Dynamic Band Splitting and Automatic Makeup Gain per Band for Mastering-grade Dynamic Control
 *
 * 功能: 实现多频段压缩器(Multiband compressor)，采用动态频段分割(dynamic band splitting)
 *       与每频段自动补偿增益(automatic makeup gain per band)实现母带级动态控制(mastering-grade dynamic control)。
 *
 * 协作: AdaptiveFilter10(自适应滤波) / SlidingDFT11(滑动DFT) / BiquadFilter9(双二阶滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(动态频段分割与每频段自动补偿增益)
 */
class MultibandCompressor10 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 10.0;       // ms
        double release = 100.0;     // ms
        double makeupGain = 0.0;    // dB (auto-computed if enabled)
        bool autoMakeup = true;
    };

    /** @brief Per-band analysis result */
    struct BandLevel {
        double inputDb = -120.0;
        double outputDb = -120.0;
        double gainReduction = 0.0;
        double makeupGain = 0.0;
    };

    /** @brief Frame output */
    struct FrameOutput {
        double sample = 0.0;
        QVector<BandLevel> bandLevels;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor10(int numBands = 4, QObject *parent = nullptr);
    ~MultibandCompressor10() override;

    void setSampleRate(double sr);
    void setNumBands(int n);
    void setBandParams(int band, const BandParams& params);
    void setCrossoverFreqs(const QVector<double>& freqs);

    /** @brief Process single sample through all bands */
    FrameOutput processSample(double sample);

    /** @brief Process block of samples */
    QVector<FrameOutput> processBlock(const QVector<double>& block);

    /** @brief Compute automatic makeup gain for all bands */
    void computeAutoMakeup();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockDone(int length, double timeMs);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<BandParams> m_params;
    QVector<double> m_crossoverFreqs;

    // Per-band state: 2nd-order crossover filters (biquad coefficients)
    struct FilterState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
    };

    struct BandState {
        QVector<FilterState> lpFilter;   // Linkwitz-Riley LP
        QVector<FilterState> hpFilter;   // Linkwitz-Riley HP
        double envDb = -120.0;           // Envelope follower (dB)
        double gainLin = 1.0;           // Current gain
        double sampleRate = 44100.0;
    };

    QVector<BandState> m_bandStates;

    /** @brief Design Linkwitz-Riley crossover for a frequency */
    void designCrossover(double freq, FilterState& lp, FilterState& hp);

    /** @brief Apply biquad filter */
    double applyFilter(FilterState& fs, double x);

    /** @brief Compute gain reduction for a band */
    double computeGainReduction(int band, double inputDb);

    /** @brief Initialize filter states */
    void initFilters();
};
