/**
 * @file Expander15.h
 * @brief 动态扩展器(多频段包络跟随与心理声学加权动态范围增强实现响度最大化) — Expander with Multiband Envelope Follower and Psychoacoustic-weighted Dynamic Range Enhancement for Loudness Maximization
 *
 * 功能: 实现动态扩展器(dynamic expander)，采用多频段包络跟随(multiband envelope follower)
 *       与心理声学加权(psychoacoustic weighting)实现动态范围增强(dynamic range enhancement)与响度最大化(loudness maximization)。
 *
 * 协作: Compressor15(压缩器) / Equalizer12(均衡器) / Limiter10(限制器)
 */
#pragma once

#include <QObject>
#include <QVector>

class Expander15 : public QObject {
    Q_OBJECT

public:
    /** @brief Band configuration for multiband processing */
    struct BandConfig {
        double lowFreq = 0.0;          // Lower crossover frequency (Hz)
        double highFreq = 0.0;         // Upper crossover frequency (Hz)
        double threshold = -30.0;      // Expansion threshold (dB)
        double ratio = 2.0;            // Expansion ratio
        double attack = 5.0;           // Attack time (ms)
        double release = 50.0;         // Release time (ms)
        double makeupGain = 0.0;       // Makeup gain (dB)
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        double gainReduction = 0.0;    // Average gain reduction (dB)
        double peakLevel = 0.0;        // Output peak level (dB)
        double loudness = 0.0;         // Perceived loudness (LUFS approx)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander15(QObject *parent = nullptr);
    ~Expander15() override;

    void setSampleRate(double sr);
    void setNumBands(int bands);
    void setBandConfig(int band, const BandConfig& config);

    /** @brief Process a block of audio samples */
    ProcessResult process(const QVector<double>& input);

    /** @brief Compute psychoacoustic weight (A-weighting approximation) for frequency */
    double psychoacousticWeight(double freq) const;

    /** @brief Compute LUFS-like loudness of signal */
    double measureLoudness(const QVector<double>& samples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, double gainRed, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<BandConfig> m_bands;
    QVector<double> m_envState;         // Envelope follower state per band
    QVector<QVector<double>> m_filterX; // Crossover filter states (biquad X)
    QVector<QVector<double>> m_filterY; // Crossover filter states (biquad Y)

    /** @brief Apply Linkwitz-Riley crossover to split into bands */
    QVector<QVector<double>> splitBands(const QVector<double>& input);

    /** @brief Apply expansion gain to one band */
    QVector<double> expandBand(const QVector<double>& band, int bandIdx);

    /** @brief Envelope follower: ballistics (attack/release) */
    double followEnvelope(double sample, double& state,
                           double attackCoeff, double releaseCoeff) const;

    /** @brief dB conversion utilities */
    static double linearToDb(double lin);
    static double dbToLinear(double db);

    /** @brief Precompute crossover filter coefficients */
    void updateFilterCoeffs();
};
