/**
 * @file Limiter8.h
 * @brief 多频带限幅器(级联前瞻+频带间能量重分配频谱平衡) — Multi-band Limiter with Cascaded Lookahead and Inter-band Energy Redistribution for Spectral Balance
 *
 * 功能: 实现多频带限幅器，采用级联前瞻(cascaded lookahead)消除瞬态失真，
 *       通过频带间能量重分配(inter-band energy redistribution)维持频谱平衡。
 *
 * 协作: Compressor4(压缩器) / Equalizer3(均衡器) / EnvelopeDetector5(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频带限幅器(级联前瞻+频带间能量重分配)
 */
class Limiter8 : public QObject {
    Q_OBJECT

public:
    /** @brief Band configuration */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -6.0;
        double attack = 1.0;
        double release = 50.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int numSamples = 0;
        double peakReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter8(int sampleRate = 44100, QObject *parent = nullptr);
    ~Limiter8() override;

    /** @brief Configure frequency bands */
    bool setBands(const QVector<BandConfig>& bands);

    /** @brief Process audio block through multi-band limiter */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample */
    double processSample(double sample);

    /** @brief Reset internal state */
    void reset();

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    int sampleRate() const { return m_sampleRate; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReduction, double timeMs);

private:
    int m_sampleRate;
    int m_numBands = 4;

    // Per-band state
    struct BandState {
        double envelope = 0.0;
        double gain = 1.0;
        double thresholdLin = 0.5;
        double attackCoeff = 0.0;
        double releaseCoeff = 0.0;
        QVector<double> lookaheadBuffer;
        int lookaheadPos = 0;
        int lookaheadSize = 0;
    };

    QVector<BandConfig> m_configs;
    QVector<BandState> m_bands;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute envelope with attack/release */
    double computeEnvelope(double input, BandState& band);

    /** @brief Redistribute energy across bands */
    void redistributeEnergy(QVector<double>& gains);

    /** @brief Apply gain with lookahead compensation */
    double applyLookahead(double input, double gain, BandState& band);
};
