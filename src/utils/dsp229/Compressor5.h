/**
 * @file Compressor5.h
 * @brief 侧链压缩器(外部键控输入+多频段依赖时序去齿音与声塑形) — Sidechain Compressor with External Key Input and Multiband Dependent Timing for De-essing and Vocal Shaping
 *
 * 功能: 实现侧链压缩器，支持外部键控(key)输入源，多频段依赖的启动/释放时间，
 *       专用于去齿音(de-essing)和人声乐声塑形(vocal shaping)。
 *
 * 协作: Equalizer3(均衡器) / Limiter2(限幅器) / AdaptiveFilter6(自适应滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 侧链压缩器(外部键控+多频段时序)
 */
class Compressor5 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -20.0;
        double ratio = 4.0;
        double attack = 5.0;
        double release = 50.0;
        double knee = 6.0;
        double makeupGain = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int framesProcessed = 0;
        double avgGainReduction = 0.0;
        double peakReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor5(QObject *parent = nullptr);
    ~Compressor5() override;

    /** @brief Set sample rate and configure frequency bands */
    bool configure(double sampleRate, const QVector<BandParams>& bands);

    /** @brief Process audio with internal (self) sidechain */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process audio with external sidechain key input */
    QVector<double> processWithKey(const QVector<double>& input,
                                    const QVector<double>& keyInput);

    /** @brief Get current gain reduction per band in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frames, double avgReduction, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 1;

    QVector<BandParams> m_bands;
    QVector<double> m_envelopeState;   // per-band envelope follower
    QVector<double> m_prevGain;        // per-band previous gain

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Convert dB to linear */
    static double dbToLinear(double db);

    /** @brief Convert linear to dB */
    static double linearToDb(double lin);

    /** @brief Soft knee gain computation */
    double computeGain(double inputDb, const BandParams& bp) const;

    /** @brief Simple 2nd-order Linkwitz-Riley crossover split */
    void crossoverSplit(const QVector<double>& input,
                         int bandIdx,
                         QVector<double>& lowOut,
                         QVector<double>& highOut) const;

    /** @brief Envelope follower with band-dependent timing */
    double followEnvelope(double inputLevel, int bandIdx);
};

