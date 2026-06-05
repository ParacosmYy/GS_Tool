/**
 * @file PingPongBuffer.h
 * @brief 乒乓缓冲区 — 双缓冲交替读写
 *
 * 功能: 乒乓(双)缓冲区，支持前台/后台交替读写，
 *       实现零拷贝切换，统计切换次数/溢出次数/耗时。
 */
#ifndef PINGPONGBUFFER_H
#define PINGPONGBUFFER_H

#include <QObject>
#include <QVector>
#include <QMutex>

class PingPongBuffer : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalSwaps = 0;
        quint64 totalWrites = 0;
        quint64 totalReads = 0;
        quint64 totalOverflows = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit PingPongBuffer(int capacity = 4096, QObject* parent = nullptr);

    /** @brief 写入数据到后台缓冲 @param data 数据 @return 实际写入数 */
    int write(const QVector<double>& data);

    /** @brief 从前台缓冲读取 @param maxCount 最大读取数 @return 读取的数据 */
    QVector<double> read(int maxCount = -1);

    /** @brief 交换前后台缓冲 */
    void swap();

    /** @brief 后台缓冲可用空间 */
    int writeAvailable() const;

    /** @brief 前台缓冲可读数据量 */
    int readAvailable() const;

    int capacity() const { return m_capacity; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bufferSwapped();
    void overflowWarning(int lostSamples);

private:
    int m_capacity;
    QVector<double> m_buffers[2];
    int m_writeBuf;
    int m_readBuf;
    int m_writePos;
    int m_readPos;
    mutable QMutex m_mutex;
    Stats m_stats;
    double m_timeSum;
};

#endif // PINGPONGBUFFER_H
