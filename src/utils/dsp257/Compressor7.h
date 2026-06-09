/**
 * @file Compressor7.h
 * @brief 多频段联动压缩器(动态EQ模式+频率依赖压缩比) — Multiband Linkage Compressor with Dynamic EQ Mode and Frequency-dependent Compression Ratio
 *
 * 功能: 实现多频段联动压缩器(Multiband Linkage Compressor)，支持动态EQ
 *       模式(dynamic EQ mode)和频率依赖压缩比(frequency-dependent
 *       compression ratio)，用于音频信号动态范围处理。
 *
 * 协作: EnvelopeDetector6(包络检测) / BiquadFilter5(双二阶滤波) / Limiter4(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段联动压缩器(动态EQ+频率依赖压缩比)
 */
class Compressor7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int sampleRate = 0;
        double inputLevel = 0.0;
        double outputLevel = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single compression band parameters */
    struct BandConfig {
        double freqLow = 0.0;
        double freqHigh = 0.0;
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 10.0;       // ms
        double release = 100.0;     // ms
        double knee = 6.0;          // dB soft knee width
        double makeupGain = 0.0;    // dB
        bool bypass = false;
    };

    /** @brief Band processing state */
    struct BandState {
        double envelope = 0.0;
        double gainReduction = 0.0;
        QVector<double> filterX1;
        QVector<double> filterX2;
        QVector<double> filterY1;
        QVector<double> filterY2;
    };

    explicit Compressor7(QObject *parent = nullptr);
    ~Compressor7() override;

    /** @brief Set sample rate */
    void setSampleRate(int rate);

    /** @brief Set number of bands */
    void setBands(int num);

    /** @brief Configure a specific band */
    void setBandConfig(int idx, const BandConfig& config);

    /** @brief Enable/disable dynamic EQ mode */
    void setDynamicEQMode(bool enabled);

    /** @brief Set linkage strength between bands (0..1) */
    void setLinkageStrength(double strength);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReductions() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void compressionCompleted(double inputLevel, double outputLevel, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_numBands = 4;
    bool m_dynamicEQ = false;
    double m_linkage = 0.5;

    QVector<BandConfig> m_configs;
    QVector<BandState> m_states;
    QVector<double> m_gainReductions;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize band filter states */
    void initFilters();

    /** @brief Design crossover filter for band */
    void designCrossover(int band, double freqLow, double freqHigh);

    /** @brief Compute envelope for band */
    double detectEnvelope(int band, double sample);

    /** @brief Compute gain reduction using soft knee */
    double computeGainReduction(double levelDB, const BandConfig& cfg) const;

    /** @brief Apply linkage across bands */
    void applyLinkage();
};
