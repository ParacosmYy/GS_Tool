/**
 * @file Expander6.h
 * @brief 多频段扩展器(对数频段划分+瞬态感知心理声学门控) — Multiband Expander with Logarithmic Band Partition and Transient-Aware Psychoacoustic Gating
 *
 * 功能: 实现多频段动态扩展器，使用对数频段划分、瞬态检测
 *       和心理声学加权门控实现高质量动态范围控制。
 *
 * 协作: MultibandGate4(多频段门控) / Compressor5(压缩器) / Goertzel7(频率检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段扩展器(对数频段+瞬态感知门控)
 */
class Expander6 : public QObject {
    Q_OBJECT

public:
    /** @brief Band configuration */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -40.0;  // dB
        double ratio = 2.0;
        double attack = 5.0;       // ms
        double release = 50.0;     // ms
        double makeupGain = 0.0;   // dB
    };

    /** @brief Band processing state */
    struct BandState {
        double envelope = 0.0;
        double gainLin = 1.0;
        double transientLevel = 0.0;
        bool gated = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int sampleRate = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander6(QObject *parent = nullptr);
    ~Expander6() override;

    /** @brief Set sample rate and block size */
    void setParameters(int sampleRate = 44100, int blockSize = 1024);

    /** @brief Configure bands with logarithmic partition */
    void configureBands(const QVector<BandConfig>& bands);

    /** @brief Auto-generate logarithmic bands from frequency range and count */
    QVector<BandConfig> generateLogBands(int numBands = 4,
                                          double minFreq = 20.0,
                                          double maxFreq = 20000.0) const;

    /** @brief Process a block of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current band states */
    QVector<BandState> bandStates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int bandActive, double peakReduction, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_blockSize = 1024;

    QVector<BandConfig> m_bands;
    QVector<BandState> m_bandStates;

    // Filter coefficients per band
    struct BiquadCoeffs {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
    };
    QVector<BiquadCoeffs> m_lowCoeffs;
    QVector<BiquadCoeffs> m_highCoeffs;

    // Filter state per band (x[n-1], x[n-2], y[n-1], y[n-2])
    struct FilterState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };
    QVector<FilterState> m_lowStates;
    QVector<FilterState> m_highStates;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute second-order Butterworth lowpass coefficients */
    BiquadCoeffs lowpassCoeffs(double cutoffFreq) const;

    /** @brief Compute second-order Butterworth highpass coefficients */
    BiquadCoeffs highpassCoeffs(double cutoffFreq) const;

    /** @brief Apply biquad filter to sample */
    double applyBiquad(double sample, const BiquadCoeffs& c, FilterState& s) const;

    /** @brief Compute envelope with attack/release ballistics */
    double computeEnvelope(double input, double attack, double release,
                           double state) const;

    /** @brief Detect transient via spectral flux */
    double detectTransient(const QVector<double>& bandSignal) const;

    /** @brief Psychoacoustic masking threshold (simplified) */
    double psychoThreshold(double freq, double level) const;
};
