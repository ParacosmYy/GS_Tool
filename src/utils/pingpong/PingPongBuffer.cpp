/**
 * @file PingPongBuffer.cpp
 * @brief 乒乓缓冲区 — 双缓冲交替读写
 */

#include "PingPongBuffer.h"
#include <QMutexLocker>
#include <QElapsedTimer>

PingPongBuffer::PingPongBuffer(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(capacity > 0 ? capacity : 4096)
    , m_writeBuf(0)
    , m_readBuf(1)
    , m_writePos(0)
    , m_readPos(0)
    , m_timeSum(0.0)
{
    m_buffers[0].resize(m_capacity);
    m_buffers[1].resize(m_capacity);
}

int PingPongBuffer::write(const QVector<double>& data)
{
    QMutexLocker locker(&m_mutex);
    QElapsedTimer timer;
    timer.start();

    int written = 0;
    for (double v : data) {
        if (m_writePos >= m_capacity) {
            m_stats.totalOverflows++;
            emit overflowWarning(data.size() - written);
            break;
        }
        m_buffers[m_writeBuf][m_writePos++] = v;
        written++;
    }

    m_stats.totalWrites++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalWrites + m_stats.totalReads + m_stats.totalSwaps);

    return written;
}

QVector<double> PingPongBuffer::read(int maxCount)
{
    QMutexLocker locker(&m_mutex);
    QElapsedTimer timer;
    timer.start();

    int available = m_capacity - m_readPos;
    int count = (maxCount < 0) ? available : qMin(maxCount, available);

    QVector<double> result(count);
    for (int i = 0; i < count; ++i) {
        result[i] = m_buffers[m_readBuf][m_readPos + i];
    }
    m_readPos += count;

    m_stats.totalReads++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalWrites + m_stats.totalReads + m_stats.totalSwaps);

    return result;
}

void PingPongBuffer::swap()
{
    QMutexLocker locker(&m_mutex);

    std::swap(m_writeBuf, m_readBuf);
    m_writePos = 0;
    m_readPos = 0;

    m_stats.totalSwaps++;
    emit bufferSwapped();
}

int PingPongBuffer::writeAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_capacity - m_writePos;
}

int PingPongBuffer::readAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_capacity - m_readPos;
}

void PingPongBuffer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
