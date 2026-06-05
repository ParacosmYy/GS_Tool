/**
 * @file MultiPeakDetector.h
 * @brief 多峰检测器 — 自适应峰值/谷值检测
 *
 * 功能: 多峰检测器，支持最小距离/最小高度/最小突出度约束，
 *       同时检测峰和谷，统计检测次数/峰值数/耗时。
 */
#ifndef MULTIPEAKDETECTOR_H
#define MULTIPEAKDETECTOR_H

#include <QObject>
#include <QVector>

class MultiPeakDetector : public QObject {
    Q_OBJECT
public:
    /** 峰/谷信息 */
    struct PeakInfo {
        int index;          ///< 位置索引
        double value;       ///< 峰/谷值
        double prominence;  ///< 突出度
    };

    /** 统计 */
    struct Stats {
        quint64 totalDetections = 0;
        quint64 totalPeaks = 0;
        quint64 totalValleys = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MultiPeakDetector(QObject* parent = nullptr);

    /** @brief 检测峰值 @param signal 输入信号 @param minDistance 最小峰间距 @param minHeight 最小峰高 @return 峰列表 */
    QVector<PeakInfo> detectPeaks(const QVector<double>& signal,
                                   int minDistance = 1,
                                   double minHeight = 0.0);

    /** @brief 检测谷值 @param signal 输入信号 @param minDistance 最小谷间距 @param maxHeight 最大谷高 @return 谷列表 */
    QVector<PeakInfo> detectValleys(const QVector<double>& signal,
                                     int minDistance = 1,
                                     double maxHeight = 0.0);

    /** @brief 计算突出度 @param signal 输入信号 @param indices 候选位置 @return 突出度数组 */
    QVector<double> computeProminence(const QVector<double>& signal,
                                       const QVector<int>& indices) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void peaksDetected(int count);
    void valleysDetected(int count);

private:
    QVector<PeakInfo> detectExtrema(const QVector<double>& signal,
                                     int minDistance, bool findPeaks) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // MULTIPEAKDETECTOR_H
