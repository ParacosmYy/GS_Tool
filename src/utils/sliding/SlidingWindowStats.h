/**
 * @file SlidingWindowStats.h
 * @brief 滑动窗口统计 — 实时计算窗口内统计特征
 *
 * 功能: O(1)时间更新均值/方差，支持多窗口大小，实时输出统计摘要。
 *
 * 协作: WindowedAggregator(窗口聚合) / DigitalFilter(预处理后统计)
 */
#ifndef SLIDINGWINDOWSTATS_H
#define SLIDINGWINDOWSTATS_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QPair>

class SlidingWindowStats : public QObject {
    Q_OBJECT
public:
    struct WindowSummary {
        double mean = 0.0;
        double variance = 0.0;
        double stddev = 0.0;
        double min = 0.0;
        double max = 0.0;
        double sum = 0.0;
        int count = 0;
    };

    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalValuesPushed = 0;
        int peakWindowSize = 0;
        double averageMean = 0.0;
    };

    explicit SlidingWindowStats(QObject* parent = nullptr);

    void setWindowSize(int size);
    void pushValue(double value);
    WindowSummary currentSummary() const;
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowUpdated(const WindowSummary& summary);

private:
    void recalculate();

    int m_windowSize;
    QQueue<double> m_window;
    WindowSummary m_current;

    Stats m_stats;
    double m_meanSum;
};

#endif // SLIDINGWINDOWSTATS_H
