/**
 * @file CircularStatsBuffer.h
 * @brief 统计环形缓冲区 — O(1)统计更新的固定容量缓冲区
 *
 * 功能: 固定容量的环形缓冲区，写入时自动覆盖最旧数据，
 *       实时维护均值/方差/最小/最大/中位数近似。
 */
#ifndef CIRCULARSTATSBUFFER_H
#define CIRCULARSTATSBUFFER_H

#include <QObject>
#include <QVector>

/**
 * @class CircularStatsBuffer
 * @brief 带统计功能的环形缓冲区
 */
class CircularStatsBuffer : public QObject {
    Q_OBJECT
public:
    /** 缓冲区统计 */
    struct Stats {
        quint64 totalWrites = 0;
        quint64 totalReads = 0;
        quint64 totalOverwrites = 0;
        int     peakUsage = 0;
        double  currentMean = 0.0;
        double  currentStddev = 0.0;
        double  currentMin = 0.0;
        double  currentMax = 0.0;
    };

    explicit CircularStatsBuffer(int capacity = 1024, QObject* parent = nullptr);

    void setCapacity(int capacity);

    /** 写入数据 */
    void write(double value);
    void writeBatch(const QVector<double>& values);

    /** 读取数据 */
    QVector<double> readAll() const;
    double readOldest() const;
    double readNewest() const;

    /** 查询 */
    int size() const;
    int capacity() const;
    bool isEmpty() const;
    bool isFull() const;

    /** 统计快照 */
    double mean() const;
    double stddev() const;
    double min() const;
    double max() const;
    double sum() const;

    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bufferFull();
    void dataOverwritten(double oldValue);
    void statsUpdated(double mean, double stddev);

private:
    void updateStats();

    QVector<double> m_buffer;
    int m_head;         ///< 写入位置
    int m_count;        ///< 当前元素数
    double m_sum;       ///< 当前总和
    double m_sqSum;     ///< 平方和
    double m_min;
    double m_max;
    Stats m_stats;
};

#endif // CIRCULARSTATSBUFFER_H
