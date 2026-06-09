/**
 * @file Expander9.h
 * @brief 多频带扩展器(独立逐频带下行扩展+智能频带间串扰抑制) — Multiband Expander with Independent Per-Band Downward Expansion and Intelligent Crosstalk Suppression Between Bands
 *
 * 功能: 实现多频带扩展器(Multiband Expander)，支持独立逐频带下行扩展(per-band
 *       downward expansion)和频带间智能串扰抑制(intelligent crosstalk
 *       suppression)，用于动态范围增强处理。
 *
 * 协作: MultibandCompressor6(多频带压缩) / LinearPredictor7(线性预测) / FftEngine(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频带扩展器(独立逐频带下行扩展+频带间串扰抑制)
 */
class Expander9 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band parameters */
    struct BandConfig {
        double lowFreq = 0.0;       // Band lower frequency (Hz)
        double highFreq = 0.0;      // Band upper frequency (Hz)
        double threshold = -40.0;   // Expansion threshold (dB)
        double ratio = 2.0;         // Expansion ratio
        double attack = 1.0;        // Attack time (ms)
        double release = 50.0;      // Release time (ms)
        double makeupGain = 0.0;    // Makeup gain (dB)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int framesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander9(int sampleRate = 44100, QObject *parent = nullptr);
    ~Expander9() override;

    /** @brief Set sample rate */
    void setSampleRate(int sr);

    /** @brief Configure frequency bands */
    void setBands(const QVector<BandConfig>& bands);

    /** @brief Set crosstalk suppression strength (0.0=none, 1.0=full) */
    void setCrosstalkSuppression(double strength);

    /** @brief Process a block of interleaved stereo samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process mono block */
    QVector<double> processMono(const QVector<double>& input);

    /** @brief Get per-band gain reduction (dB) for last processed block */
    QVector<double> bandGainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_xtalkSuppress = 0.5;

    QVector<BandConfig> m_bands;

    /** @brief Per-band state for envelope follower */
    struct BandState {
        QVector<double> envelope;       // Current envelope level per band
        QVector<double> gainReduction;  // Current gain reduction (linear)
        QVector<double> prevGain;       // Previous gain for smoothing
    };

    BandState m_state;

    /** @brief Per-band filter coefficients (Linkwitz-Riley 4th-order) */
    struct FilterCoeffs {
        double b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0;
        double a1 = 0, a2 = 0, a3 = 0, a4 = 0;
    };

    /** @brief Filter state (direct form II transposed) */
    struct FilterState {
        double w1 = 0, w2 = 0, w3 = 0, w4 = 0;
    };

    QVector<FilterCoeffs> m_lowCoeffs;
    QVector<FilterCoeffs> m_highCoeffs;
    QVector<FilterState> m_lowState;
    QVector<FilterState> m_highState;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design Linkwitz-Riley crossover filter */
    FilterCoeffs designLR4(double freq) const;

    /** @brief Apply 4th-order filter to sample */
    double applyFilter(double sample, const FilterCoeffs& c, FilterState& s) const;

    /** @brief Compute expansion gain for given level and band config */
    double expandGain(double levelDb, const BandConfig& cfg) const;

    /** @brief Compute time-constant coefficient */
    double timeCoeff(double timeMs) const;

    /** @brief Apply crosstalk suppression between bands */
    void suppressCrosstalk(QVector<QVector<double>>& bandSignals);
};
