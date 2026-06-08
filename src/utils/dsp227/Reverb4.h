/**
 * @file Reverb4.h
 * @brief 算法混响(FDN8反馈延迟网络+Householder反射矩阵8通道扩散) — Algorithmic Reverb with FDN8 Feedback Delay Network and Householder Reflection Matrix for 8-Channel Diffusion
 *
 * 功能: 实现基于FDN8(8通道反馈延迟网络)的算法混响，
 *       使用Householder反射矩阵实现8通道信号扩散与能量守恒。
 *
 * 协作: Reverb3(Schroeder混响) / IIRFilter3(IIR滤波) / SignalGenerator5(信号发生)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 算法混响(FDN8+Householder扩散)
 */
class Reverb4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numProcessed = 0;
        int numChannels = 8;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb4(QObject *parent = nullptr);
    ~Reverb4() override;

    /** @brief Configure reverb parameters */
    void setParameters(double roomSize, double damping,
                       double wetLevel = 0.5, double dryLevel = 0.5);

    /** @brief Process a block of mono samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset delay lines to zero state */
    void reset();

    /** @brief Get current tail response (for visualization) */
    QVector<double> tailResponse(int length) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    static const int kChannels = 8;
    static const int kDelayLengths[kChannels];

    double m_roomSize = 0.7;
    double m_damping = 0.5;
    double m_wetLevel = 0.5;
    double m_dryLevel = 0.5;

    // Delay line buffers
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_delayPos;

    // Feedback gains per channel
    double m_feedback[kChannels];
    // Damping state (one-pole low-pass per channel)
    double m_dampState[kChannels];

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply 8x8 Householder reflection matrix for diffusion */
    void applyHouseholder(double samples[kChannels]) const;

    /** @brief Read from delay line with interpolation */
    double readDelay(int ch) const;

    /** @brief Write to delay line */
    void writeDelay(int ch, double value);
};
