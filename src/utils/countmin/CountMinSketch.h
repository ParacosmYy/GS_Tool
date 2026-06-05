/**
 * @file CountMinSketch.h
 * @brief Count-Min Sketch — 频率近似计数
 *
 * 功能: 概率数据结构，近似统计元素频率，支持点查询/Top-K/
 *       内积估计，空间O(d*w)，统计更新/查询次数/耗时。
 */
#ifndef COUNTMINSKETCH_H
#define COUNTMINSKETCH_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QPair>

class CountMinSketch : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit CountMinSketch(int depth = 5, int width = 1000,
                             QObject* parent = nullptr);

    /** @brief 更新元素计数 @param item 元素 @param count 增量 */
    void update(const QByteArray& item, quint64 count = 1);

    /** @brief 查询元素近似频率 @param item 元素 @return 近似频率(上界) */
    quint64 estimate(const QByteArray& item) const;

    /** @brief 两个sketch的内积 @param other 另一个sketch @return 内积近似 */
    quint64 innerProduct(const CountMinSketch& other) const;

    /** @brief 合并另一个sketch @param other 另一个sketch */
    void merge(const CountMinSketch& other);

    int depth() const { return m_depth; }
    int width() const { return m_width; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void reset();

signals:
    void updated(const QByteArray& item, quint64 count);

private:
    quint32 hashItem(const QByteArray& item, int row) const;

    int m_depth;
    int m_width;
    QVector<QVector<quint64>> m_table;
    Stats m_stats;
    double m_timeSum;
};

#endif // COUNTMINSKETCH_H
