/**
 * @file Limiter15.h
 * @brief 多段前瞻限幅器(带间增益链接与ISP真峰值检测实现母带级响度管理) — Multi-Band Lookahead Limiter with Inter-Band Gain Linking and ISP True-Peak Detection for Mastering-Grade Loudness Management
 *
 * 功能: 实现多段前瞻限幅器(multi-band lookahead limiter)，采用带间增益链接(inter-band gain linking)
 *       与ISP真峰值检测(ISP true-peak detection)实现母带级响度管理(mastering-grade loudness management)。
 *
 * 协作: Compressor(压缩器) / Equalizer(均衡器) / LoudnessMeter(响度计)
 */
#pragma once

#include <QObject>
#include <QVector>

class Limiter15 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex number for FFT-based processing */
    struct Complex {
        double real = 0.0;
        double imag = 0.0;
    };

    /** @brief Single band configuration */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -1.0;    // dB
        double ceiling = -0.3;       // dB
        double attack = 1.0;         // ms
        double release = 50.0;       // ms
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        double peakReduction = 0.0;  // dB
        double truePeak = 0.0;       // dBTP
        double gainReduction = 0.0;  // dB
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        double avgGainReduction = 0.0;
        double maxTruePeak = -999.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter15(int numBands = 4, QObject *parent = nullptr);
    ~Limiter15() override;

    void setSampleRate(double rate);
    void setLookaheadSamples(int samples);
    void setBands(const QVector<BandConfig>& bands);
    void setLinkStrength(double strength);

    /** @brief Process audio frame with multi-band limiting */
    ProcessResult process(const QVector<double>& input);

    /** @brief ISP true-peak detection via 4x oversampling */
    double detectTruePeak(const QVector<double>& signal) const;

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, double truePeak, double gr, double timeMs);

private:
    int m_numBands;
    double m_sampleRate = 44100.0;
    int m_lookahead = 64;
    double m_linkStrength = 0.5;
    QVector<BandConfig> m_bands;
    Stats m_stats;
    double m_grSum = 0.0;
    double m_timeSum = 0.0;

    // Per-band state
    QVector<QVector<double>> m_delayLines;
    QVector<double> m_envelopeState;
    QVector<QVector<double>> m_crossoverCoeffs;

    /** @brief Link-squared sum for inter-band gain coupling */
    QVector<double> computeLinkedGain(const QVector<double>& bandPeaks) const;

    /** @brief Envelope follower with attack/release */
    double followEnvelope(double input, double state,
                          double attackCoeff, double releaseCoeff) const;

    /** @brief Apply crossover filter to split bands */
    QVector<QVector<double>> splitBands(const QVector<double>& input);

    /** @brief Sinc interpolation for ISP true-peak */
    double sincInterp(const QVector<double>& sig, double pos) const;
};
