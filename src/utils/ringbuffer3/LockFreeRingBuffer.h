/**
 * @file LockFreeRingBuffer.h
 * @brief 无锁环形缓冲区 — 单生产者单消费者零分配
 *
 * 功能: 基于std::atomic的无锁SPSC环形缓冲区，
 *       支持批量读写/peek/可用空间查询，
 *       统计读写次数/溢出次数/吞吐量。
 */
#ifndef LOCKFREERINGBUFFER_H
#define LOCKFREERINGBUFFER_H

#include <QObject>
#include <QVector>
#include <atomic>

class LockFreeRingBuffer : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalWrites = 0;
        quint64 totalReads = 0;
        quint64 totalOverflows = 0;
        quint64 totalBytesWritten = 0;
        quint64 totalBytesRead = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit LockFreeRingBuffer(int capacity = 4096,
                                 QObject* parent = nullptr);
    ~LockFreeRingBuffer() override;

    /** @brief 写入数据 @param data 数据指针 @param len 长度 @return 实际写入字节数 */
    int write(const char* data, int len);

    /** @brief 读取数据 @param data 输出缓冲区 @param len 最大长度 @return 实际读取字节数 */
    int read(char* data, int len);

    /** @brief 查看数据(不消费) @param data 输出缓冲区 @param len 最大长度 @return 实际查看字节数 */
    int peek(char* data, int len) const;

    /** @brief 可读字节数 */
    int availableRead() const;

    /** @brief 可写字节数 */
    int availableWrite() const;

    int capacity() const { return m_capacity; }
    bool isEmpty() const;
    bool isFull() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void overflowOccurred(int requestedBytes);
    void dataWritten(int bytesWritten);

private:
    char* m_buffer;
    int m_capacity;
    int m_mask;
    std::atomic<int> m_readPos;
    std::atomic<int> m_writePos;
    Stats m_stats;
    double m_timeSum;
};

#endif // LOCKFREERINGBUFFER_H
