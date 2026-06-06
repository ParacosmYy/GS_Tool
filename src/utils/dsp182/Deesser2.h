/**
 * @file Deesser2.h
 * @brief 去齿音处理器(频谱质心嘶嘶音检测+分频带处理+自适应阈值) — De-esser with Sibilance Detection via Spectral Centroid, Split-band Processing and Auto-threshold
 *
 * 功能: 实现去齿音(DA处理)，支持基于频谱质心的嘶嘶音检测、
 *       分频带处理(高频压缩)和自适应阈值自动调节。
 *
 * 协作: Compressor5(压缩器) / EQ3(均衡器) / Gate6(门限)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音处理器(频谱质心检测+分频带压缩)
 */
class Deesser2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int sibilanceDetections = 0;
        double avgReductionDb = 0.0;
        double spectralCentroidHz = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser2(QObject *parent = nullptr);
    ~Deesser2() override;

    void setThreshold(double threshDb);
    void setFrequency(double freqHz);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setAutoThreshold(bool enabled);
    void setSampleRate(double rate);

    /** @brief 处理音频帧，返回处理后数据 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 计算频谱质心(Hz) */
    double spectralCentroid(const QVector<double>& frame) const;

    /** @brief 计算RMS能量 */
    double rmsEnergy(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(int frame, double centroidHz, double reductionDb);

private:
    double m_thresholdDb = -20.0;
    double m_frequency = 6000.0;     ///< Crossover frequency in Hz
    double m_ratio = 4.0;
    double m_attackMs = 0.5;
    double m_releaseMs = 50.0;
    bool m_autoThreshold = false;
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    double m_envelope = 0.0;         ///< Current gain envelope
    double m_phase = 0.0;            ///< Oscillator phase for crossover

    /** @brief First-order crossover: split into high and low bands */
    void splitBand(const QVector<double>& input,
                   QVector<double>& low, QVector<double>& high);

    /** @brief Merge high and low bands back */
    QVector<double> mergeBand(const QVector<double>& low, const QVector<double>& high) const;

    /** @brief Estimate adaptive threshold from signal level */
    double estimateThreshold(const QVector<double>& frame) const;
};
