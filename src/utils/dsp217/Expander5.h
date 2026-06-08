/**
 * @file Expander5.h
 * @brief 多频段扩展器(心理声学加权阈值+瞬态保护释放曲线) — Multiband Expander with Psychoacoustic-Weighted Threshold and Transient-Preserving Release Curve
 *
 * 功能: 实现多频段动态范围扩展器，采用心理声学模型计算频段阈值，
 *       瞬态保护释放曲线避免信号失真，支持可配置频段数和斜率。
 *
 * 协作: Compressor4(压缩器) / Limiter3(限幅器) / Equalizer4(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段扩展器(心理声学阈值+瞬态保护)
 */
class Expander5 : public QObject {
    Q_OBJECT

public:
    /** @brief Band configuration */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -40.0;    // dB
        double ratio = 2.0;
        double attack = 1.0;         // ms
        double release = 50.0;       // ms
        double makeupGain = 0.0;     // dB
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int frameSize = 0;
        double avgInputLevel = 0.0;
        double avgOutputLevel = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander5(QObject *parent = nullptr);
    ~Expander5() override;

    /** @brief Set band configurations */
    void setBands(const QVector<BandConfig>& bands);

    /** @brief Set psychoacoustic weighting mode: 0=none, 1=A-weight, 2=ISO226 */
    void setPsychoacousticMode(int mode);

    /** @brief Process single frame of interleaved samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double avgGainReduction, double timeMs);

private:
    int m_psychoMode = 1;
    int m_frameSize = 512;

    QVector<BandConfig> m_bands;

    // Per-band envelope state
    QVector<double> m_envelope;
    QVector<double> m_gainReduction;
    QVector<double> m_prevTransient;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply linkwitz-riley crossover filter */
    void crossoverFilter(const QVector<double>& input,
                          QVector<QVector<double>>& bandOutputs);

    /** @brief Compute psychoacoustic threshold for frequency */
    double psychoWeight(double freq) const;

    /** @brief Compute gain with transient-preserving release */
    double computeGain(double inputDb, double threshold,
                        double ratio, int bandIdx);

    /** @brief Envelope follower with adaptive release */
    double envelopeFollow(double input, double attack,
                           double release, int bandIdx);

    /** @brief Transient detection for release curve protection */
    bool isTransient(double input, int bandIdx) const;
};
