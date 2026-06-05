/**
 * @file MovingMinMax.h
 * @brief 滑动窗口最大/最小值 — O(1)单调双端队列
 *
 * 功能: 使用单调双端队列实现O(1)窗口最大/最小值查询，
 *       支持批量处理，统计更新次数/查询次数/耗时。
 */
#ifndef MOVINGMINMAX_H
#define MOVINGMINMAX_H

#include <QObject>
#include <QVector>
#include <deque>

class MovingMinMax : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MovingMinMax(int windowSize = 50, QObject* parent = nullptr);

    /** @brief 添加值 @param value 新值 */
    void add(double value);

    /** @brief 当前窗口最大值 @return 最大值 */
    double max() const;

    /** @brief 当前窗口最小值 @return 最小值 */
    double min() const;

    /** @brief 批量处理并返回每窗口最大值 @param values 数据 @return 每个窗口结束时的最大值 */
    QVector<double> batchMax(const QVector<double>& values);

    /** @brief 批量处理并返回每窗口最小值 @param values 数据 @return 每个窗口结束时的最小值 */
    QVector<double> batchMin(const QVector<double>& values);

    int windowSize() const { return m_windowSize; }
    int count() const { return m_buffer.size(); }

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowUpdated(double minVal, double maxVal);

private:
    int m_windowSize;
    QVector<double> m_buffer;
    std::deque<int> m_maxDeque;  ///< 递减队列(存索引)
    std::deque<int> m_minDeque;  ///< 递增队列(存索引)
    int m_pos;
    Stats m_stats;
    double m_timeSum;
};

#endif // MOVINGMINMAX_H
