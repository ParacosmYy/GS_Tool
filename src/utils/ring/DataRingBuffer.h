/**
 * @file DataRingBuffer.h
 * @brief 数据环形缓冲区 -- 线程安全的循环字节缓冲区
 */

#ifndef DATARINGBUFFER_H
#define DATARINGBUFFER_H

#include <QByteArray>
#include <QObject>
#include <QReadWriteLock>

class DataRingBuffer : public QObject {
    Q_OBJECT

public:
    enum class OverflowPolicy { OverwriteOldest, DropNewest, ExpandCapacity };

    struct Stats {
        quint64 totalBytesWritten = 0;
        quint64 totalBytesRead = 0;
        quint64 totalOverflows = 0;
        quint64 totalReadUnderruns = 0;
        quint64 totalWrites = 0;
        quint64 totalReads = 0;
        double avgWriteSize = 0.0;
        double avgReadSize = 0.0;
        double utilization = 0.0;
    };

    explicit DataRingBuffer(int capacity = 4096, QObject* parent = nullptr);
    ~DataRingBuffer() override;

    void setCapacity(int bytes);
    int capacity() const;
    void setOverflowPolicy(OverflowPolicy policy);
    OverflowPolicy overflowPolicy() const;
    int write(const QByteArray& data);
    QByteArray read(int maxBytes);
    QByteArray peek(int maxBytes) const;
    int skip(int bytes);
    int available() const;
    int freeSpace() const;
    bool isEmpty() const;
    bool isFull() const;
    void clear();
    Stats stats() const;
    void resetStatistics();

signals:
    void dataWritten(int bytes);
    void dataRead(int bytes);
    void overflow(int droppedBytes);
    void bufferEmpty();
    void bufferFull();

private:
    QByteArray m_buffer;
    int m_capacity;
    int m_head = 0;
    int m_tail = 0;
    int m_count = 0;
    OverflowPolicy m_policy = OverflowPolicy::OverwriteOldest;
    mutable QReadWriteLock m_lock;
    Stats m_stats;
    quint64 m_sumWriteSize = 0;
    quint64 m_sumReadSize = 0;
    void updateUtilization();
};

#endif // DATARINGBUFFER_H
