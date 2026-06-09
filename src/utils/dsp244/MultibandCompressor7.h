/**
 * @file MultibandCompressor7.h
 * @brief 多频段压缩器(Linkwitz-Riley交叉+独立频段阈值/比率/补偿增益) — Multiband Compressor with Linkwitz-Riley Crossover and Independent Per-Band Threshold/Ratio/Makeup Gain
 *
 * 功能: 实现多频段动态范围压缩器(multiband compressor)，使用Linkwitz-Riley
 *       交叉滤波器(Linkwitz-Riley crossover)进行频段分割，每个频段具有独立的
 *       阈值(threshold)、压缩比率(ratio)和补偿增益(makeup gain)参数。
 *
 * 协作: FIRFilter6(FIR滤波) / IIRFilter7(IIR滤波) / Limiter5(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(Linkwitz-Riley交叉+独立频段阈值/比率/补偿增益)
 */
class MultibandCompressor7 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;         // compression ratio
        double attack = 10.0;       // ms
        double release = 100.0;     // ms
        double makeupGain = 0.0;    // dB
        double kneeWidth = 6.0;     // dB soft knee width
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor7(QObject *parent = nullptr);
    ~MultibandCompressor7() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set crossover frequencies between bands (n-1 freqs for n bands) */
    void setCrossoverFreqs(const QVector<double>& freqs);

    /** @brief Set parameters for a specific band */
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a block of samples, returns compressed output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process a single sample */
    double processOne(double sample);

    /** @brief Get per-band gain reduction in dB (after last process call) */
    QVector<double> gainReductions() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int numSamples, int numBands, double timeMs);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;
    QVector<double> m_crossoverFreqs;

    /** @brief Per-band state */
    struct BandState {
        BandParams params;
        // Linkwitz-Riley 4th-order filter state (2 cascaded 2nd-order)
        double x1[2] = {0, 0}, x2[2] = {0, 0};  // input delays per stage
        double y1[2] = {0, 0}, y2[2] = {0, 0};  // output delays per stage
        // Low-pass and high-pass for each crossover
        double lpX1[2] = {0, 0}, lpX2[2] = {0, 0};
        double lpY1[2] = {0, 0}, lpY2[2] = {0, 0};
        double hpX1[2] = {0, 0}, hpX2[2] = {0, 0};
        double hpY1[2] = {0, 0}, hpY2[2] = {0, 0};
        // Envelope follower
        double envelope = 0.0;
        double gainReduction = 0.0;
        // Filter coefficients
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
        double hpb0 = 1.0, hpb1 = 0.0, hpb2 = 0.0;
        double hpa1 = 0.0, hpa2 = 0.0;
        bool filterReady = false;
    };

    QVector<BandState> m_bands;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Linkwitz-Riley 4th-order coefficients for a crossover */
    void computeLRCoeffs(BandState& band, double freq);

    /** @brief Apply low-pass filter to a sample for one band */
    double applyLP(BandState& band, double x);

    /** @brief Apply high-pass filter to a sample for one band */
    double applyHP(BandState& band, double x);

    /** @brief Compute gain reduction for one band */
    double computeGain(BandState& band, double input);

    /** @brief Initialize band filters from crossover frequencies */
    void initFilters();
};
