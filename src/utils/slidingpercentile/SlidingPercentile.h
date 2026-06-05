/**
 * @file SlidingPercentile.h
 * @brief 滑动百分位计算器 — O(log n)窗口百分位
 *
 * 功能: 基于索引跳表的滑动窗口百分位计算，支持多百分位查询，
 *       统计查询次数/窗口大小/耗时。
 */
#ifndef SLIDINGPERCENTILE_H
#define SLIDINGPERCENTILE_H

#include <QObject>
#include <QVector>
#include <QMap>

class SlidingPercentile : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SlidingPercentile(int windowSize = 1000,
                                QObject* parent = nullptr);

    /** @brief 添加值 @param value 新值 */
    void add(double value);

    /** @brief 查询百分位 @param p 百分位(0~100) @return 值 */
    double query(double p) const;

    /** @brief 查询多个百分位 @param percentiles 百分位列表 @return 值列表 */
    QVector<double> queryBatch(const QVector<double>& percentiles) const;

    /** @brief 中位数 @return 中位数值 */
    double median() const { return query(50.0); }

    /** @brief 四分位距 @return IQR */
    double iqr() const { return query(75.0) - query(25.0); }

    int windowSize() const { return m_windowSize; }
    int count() const { return m_buffer.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueAdded(double value, double p50);

private:
    int m_windowSize;
    QVector<double> m_buffer;
    QMap<double, int> m_sorted;  ///< 值→计数
    int m_pos;
    Stats m_stats;
    double m_timeSum;
};

#endif // SLIDINGPERCENTILE_H
