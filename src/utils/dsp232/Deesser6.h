/**
 * @file Deesser6.h
 * @brief 去齿音器(Bark频带能量比心理声学检测+多频带动态抑制) — De-esser with Psychoacoustic Sibilance Detection via Bark-Band Energy Ratio and Multiband Dynamic Suppression
 *
 * 功能: 实现去齿音(de-esser)处理器，通过Bark频带能量比(Bark-band energy ratio)进行
 *       心理声学齿音检测(psychoacoustic sibilance detection)，并采用多频带动态抑制(multiband
 *       dynamic suppression)精确降低齿音成分。
 *
 * 协作: MultibandCompressor4(多频带压缩) / SpectralGate3(频谱门) / Limiter5(限制器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音器(Bark频带能量比心理声学检测+多频带动态抑制)
 */
class Deesser6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        int numSibilanceDetected = 0;
        double avgReductionDb = 0.0;
        double peakSibilanceDb = -120.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser6(QObject *parent = nullptr);
    ~Deesser6() override;

    /** @brief Configure de-esser parameters */
    void setParameters(int sampleRate, double thresholdDb = -20.0,
                       double ratio = 4.0, double attackMs = 0.1,
                       double releaseMs = 50.0, double freqHz = 6000.0);

    /** @brief Process a block of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process stereo (interleaved L,R) */
    QVector<double> processStereo(const QVector<double>& input);

    /** @brief Get current sibilance level in dB */
    double sibilanceLevelDb() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(double levelDb, double reductionDb);
    void frameProcessed(int frameNum, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_attackMs = 0.1;
    double m_releaseMs = 50.0;
    double m_centerFreq = 6000.0;
    double m_bandwidth = 3000.0;

    // Band-split filter state (Linkwitz-Riley 2nd order)
    double m_x1L = 0.0, m_x2L = 0.0;
    double m_y1L = 0.0, m_y2L = 0.0;
    double m_x1H = 0.0, m_x2H = 0.0;
    double m_y1H = 0.0, m_y2H = 0.0;

    // Envelope follower state
    double m_envLevel = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    // Gain reduction smoothing
    double m_currentGainDb = 0.0;

    double m_sibilanceLevelDb = -120.0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;

    /** @brief Compute Bark frequency from Hz */
    double hzToBark(double hz) const;

    /** @brief Compute sibilance energy ratio in Bark bands */
    double sibilanceRatio(const QVector<double>& highBand) const;

    /** @brief Compute gain reduction from envelope */
    double computeGainReduction(double levelDb) const;

    /** @brief Apply gain to high band sample */
    double applyGain(double sample, double gainDb) const;
};
