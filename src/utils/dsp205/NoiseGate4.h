/**
 * @file NoiseGate4.h
 * @brief 噪声门(自适应谱底最小统计噪声估计) — Noise Gate with Adaptive Spectral Floor via Minimum-Statistics Noise Estimation
 *
 * 功能: 实现自适应噪声门，使用最小统计方法估计噪声谱底，
 *       支持谱减法降噪、实时噪声跟踪和阈值自适应调整。
 *
 * 协作: WienerFilter4(维纳滤波) / SpectralSubtraction5(谱减法) / Goertzel6(频率检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(自适应谱底最小统计噪声估计)
 */
class NoiseGate4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        double avgNoiseFloor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate4(QObject *parent = nullptr);
    ~NoiseGate4() override;

    void setThreshold(double dB);
    void setAttack(int samples);
    void setRelease(int samples);
    void setHistoryFrames(int frames);

    /** @brief Process a single frame, return gated output */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Estimate noise floor using minimum statistics */
    double estimateNoiseFloor(const QVector<double>& powerSpectrum);

    /** @brief Update noise estimate with minimum statistics tracking */
    void updateNoiseEstimate(const QVector<double>& powerSpectrum);

    /** @brief Apply spectral floor gating */
    QVector<double> applyGate(const QVector<double>& frame,
                               const QVector<double>& noiseFloor) const;

    /** @brief Get current noise estimate */
    QVector<double> noiseEstimate() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double noiseFloor, double timeMs);

private:
    double m_thresholdDb = -40.0;
    int m_attack = 64;
    int m_release = 256;
    int m_historyLen = 8;

    QVector<double> m_noiseEstimate;
    QVector<QVector<double>> m_powerHistory;
    int m_historyIdx = 0;

    double m_gain = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute power spectrum of a frame */
    static QVector<double> computePower(const QVector<double>& frame);
};
