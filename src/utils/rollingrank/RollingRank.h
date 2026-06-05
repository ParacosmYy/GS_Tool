/**
 * @file RollingRank.h
 * @brief 滚动排名计算器 — 滑动窗口内百分位排名
 *
 * 功能: 维护滑动窗口，实时计算指定值在窗口内的百分位排名，
 *       支持多窗口查询，统计计算次数/耗时。
 */
#ifndef ROLLINGRANK_H
#define ROLLINGRANK_H

#include <QObject>
#include <QVector>
#include <QMap>

class RollingRank : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RollingRank(int windowSize = 100, QObject* parent = nullptr);

    /** @brief 添加数据点 @param value 值 */
    void add(double value);

    /** @brief 查询当前值在窗口中的百分位排名(0~100) @param value 查询值 @return 百分位 */
    double percentileRank(double value) const;

    /** @brief 查询窗口内指定百分位的值 @param p 百分位(0~100) @return 值 */
    double percentile(double p) const;

    /** @brief 窗口内中位数 @return 中位数 */
    double median() const;

    /** @brief 窗口内排名 @param value 值 @return 排名(1-based) */
    int rank(double value) const;

    int windowSize() const { return m_windowSize; }
    int count() const { return m_window.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rankUpdated(double value, double rank);

private:
    int m_windowSize;
    QVector<double> m_window;
    Stats m_stats;
    double m_timeSum;
};

#endif // ROLLINGRANK_H
