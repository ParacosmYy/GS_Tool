/**
 * @file SignalSynchronizer4.h
 * @brief 信号同步器(交叉熵时延估计+加权图对齐) — Signal Synchronizer with Cross-Entropy Time-Delay Estimation and Weighted Graph Alignment
 *
 * 功能: 实现多通道信号同步，支持交叉熵时延估计、
 *       加权图对齐和动态时间规整。
 *
 * 协作: CrossCorrelation3(互相关) / FFT5(FFT) / SignalFilter3(信号滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号同步器(交叉熵时延估计+加权图对齐)
 */
class SignalSynchronizer4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSignals = 0;
        int signalLength = 0;
        int estimatedDelay = 0;
        double syncQuality = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer4(QObject *parent = nullptr);
    ~SignalSynchronizer4() override;

    /** @brief Set search range for delay estimation */
    void setSearchRange(int maxDelay);

    /** @brief Estimate delay between reference and signal via cross-entropy */
    int estimateDelay(const QVector<double>& reference,
                      const QVector<double>& signal) const;

    /** @brief Estimate delays for multiple signals against a reference */
    QVector<int> estimateDelays(
        const QVector<double>& reference,
        const QVector<QVector<double>>& signals) const;

    /** @brief Synchronize signals by shifting to align with reference */
    QVector<QVector<double>> synchronize(
        const QVector<QVector<double>>& signals,
        int referenceIndex = 0);

    /** @brief Build weighted alignment graph from delay estimates */
    QVector<QVector<QPair<int, double>>> buildAlignmentGraph(
        const QVector<QVector<double>>& signals) const;

    /** @brief Find optimal global alignment via graph shortest paths */
    QVector<int> globalAlignment(
        const QVector<QVector<QPair<int, double>>>& graph,
        int numSignals) const;

    /** @brief Compute cross-entropy between two signal segments */
    static double crossEntropy(const QVector<double>& a,
                                const QVector<double>& b);

    /** @brief Align two signals using DTW */
    QVector<QPair<int, int>> dtwAlignment(
        const QVector<double>& a, const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void synchronizationCompleted(int numSignals, double quality, double timeMs);

private:
    int m_maxDelay = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Normalize signal to probability distribution */
    static QVector<double> normalizeToProb(const QVector<double>& signal);

    /** @brief Shift signal by given delay (positive = right shift) */
    static QVector<double> shiftSignal(
        const QVector<double>& signal, int delay);
};
