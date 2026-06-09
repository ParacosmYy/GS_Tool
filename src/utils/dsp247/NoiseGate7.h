/**
 * @file NoiseGate7.h
 * @brief 噪声门(信号直方图自适应阈值估计+迟滞控制开关) — Noise Gate with Adaptive Threshold Estimation via Signal Histogram Analysis and Hysteresis-Controlled Open/Close
 *
 * 功能: 实现噪声门(Noise Gate)，通过信号直方图分析(signal histogram
 *       analysis)自适应估计噪声阈值，使用迟滞控制(hysteresis-controlled)
 *       的开启/关闭阈值避免门抖动(gate chattering)。
 *
 * 协作: NoiseProfile6(噪声分析) / Compressor7(动态压缩) / Expander6(动态扩展)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(直方图自适应阈值+迟滞控制开关)
 */
class NoiseGate7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        double openThreshold = 0.0;
        double closeThreshold = 0.0;
        int numOpenings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate7(QObject *parent = nullptr);
    ~NoiseGate7() override;

    /** @brief Set open threshold manually (0 = auto-estimate) */
    void setOpenThreshold(double threshold);

    /** @brief Set hysteresis width (close = open - hysteresis) */
    void setHysteresis(double hysteresis);

    /** @brief Set attack time in samples */
    void setAttack(int samples);

    /** @brief Set release time in samples */
    void setRelease(int samples);

    /** @brief Set hold time in samples (minimum open duration) */
    void setHold(int samples);

    /** @brief Set reduction gain when gate is closed (0.0 = mute) */
    void setClosedGain(double gain);

    /** @brief Process signal through noise gate */
    QVector<double> process(const QVector<double>& input);

    /** @brief Auto-estimate threshold from signal histogram */
    double estimateThreshold(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int length, double threshold, double timeMs);

private:
    double m_openThreshold = 0.0;   // 0 = auto
    double m_hysteresis = 0.05;
    int m_attack = 10;
    int m_release = 100;
    int m_hold = 50;
    double m_closedGain = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build signal amplitude histogram */
    QVector<int> buildHistogram(const QVector<double>& input,
                                 int numBins, double& minVal, double& maxVal) const;

    /** @brief Find valley in histogram for noise floor estimation */
    double findHistogramValley(const QVector<int>& bins,
                                int numBins, double minVal, double maxVal) const;

    /** @brief One-pole smoother coefficient */
    static double smoothCoeff(int timeSamples);
};
