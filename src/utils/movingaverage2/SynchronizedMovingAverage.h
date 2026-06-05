/**
 * @file SynchronizedMovingAverage.h
 * @brief 线程安全移动平均 — QMutex保护的滑动窗口均值
 *
 * 功能: 线程安全的滑动窗口均值/方差计算，
 *       支持加权平均，统计更新次数/耗时。
 */
#ifndef SYNCHRONIZEDMOVINGAVERAGE_H
#define SYNCHRONIZEDMOVINGAVERAGE_H

#include <QObject>
#include <QVector>
#include <QMutex>

class SynchronizedMovingAverage : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SynchronizedMovingAverage(int windowSize = 50,
                                        QObject* parent = nullptr);

    /** @brief 添加值(线程安全) @param value 新值 */
    void add(double value);

    /** @brief 当前均值(线程安全) @return 均值 */
    double mean() const;

    /** @brief 当前方差(线程安全) @return 方差 */
    double variance() const;

    /** @brief 当前标准差 @return 标准差 */
    double stdDev() const;

    /** @brief 当前中位数(线程安全) @return 中位数 */
    double median() const;

    int windowSize() const { return m_windowSize; }
    int count() const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void valueAdded(double value, double mean);

private:
    int m_windowSize;
    QVector<double> m_buffer;
    int m_pos;
    int m_count;
    double m_sum;
    mutable QMutex m_mutex;
    Stats m_stats;
    double m_timeSum;
};

#endif // SYNCHRONIZEDMOVINGAVERAGE_H
