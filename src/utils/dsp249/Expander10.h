/**
 * @file Expander10.h
 * @brief 多频段扩展器(对数频段分割+独立时间常数逐频段处理) — Multiband Expander with Logarithmic Band Splitting and Independent Time-Constant Per Frequency Region
 *
 * 功能: 实现多频段扩展器(Multiband Expander)，通过对数频段分割
 *       (logarithmic band splitting)将频谱划分为多个子带，每个子带
 *       拥有独立的攻击/释放时间常数(independent time constant)进行
 *       增益控制，实现噪声门限与动态范围扩展。
 *
 * 协作: Compressor10(动态压缩器) / Equalizer10(均衡器) / FilterDesign7(滤波器设计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段扩展器(对数分割+独立时间常数)
 */
class Expander10 : public QObject {
    Q_OBJECT

public:
    /** @brief Band configuration */
    struct BandConfig {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double threshold = -40.0;    // dB
        double ratio = 2.0;
        double attack = 5.0;         // ms
        double release = 50.0;       // ms
    };

    /** @brief Per-band level measurement */
    struct BandLevel {
        double inputDb = -120.0;
        double outputDb = -120.0;
        double gainDb = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander10(QObject *parent = nullptr);
    ~Expander10() override;

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Configure bands with logarithmic splitting */
    void setBands(const QVector<BandConfig>& bands);

    /** @brief Process a block of samples through all bands */
    QVector<double> processBlock(const QVector<double>& samples);

    /** @brief Get per-band level readings after last process */
    QVector<BandLevel> bandLevels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int blockSize, int numBands, double timeMs);

private:
    double m_sampleRate = 44100.0;

    /** @brief Internal band state */
    struct BandState {
        BandConfig config;
        double envelope = 0.0;       // Current envelope follower
        double gainLinear = 1.0;     // Current gain
        QVector<double> filterCoeff; // Simple bandpass coefficients
        BandLevel level;
    };

    QVector<BandState> m_bands;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute gain from envelope and band config */
    double computeGain(double inputDb, const BandConfig& cfg) const;

    /** @brief Update envelope follower with attack/release */
    double updateEnvelope(double input, double envelope,
                          double attackCoeff, double releaseCoeff) const;

    /** @brief Compute logarithmic crossover frequencies */
    QVector<QPair<double, double>> logBandEdges(int numBands,
                                                 double minFreq, double maxFreq) const;

    /** @brief Simple first-order bandpass filter coefficient */
    double bandpassCoeff(double lowFreq, double highFreq, double sampleRate) const;
};
