/**
 * @file DataRingBuffer.cpp
 * @brief 数据环形缓冲区实现 -- 线程安全循环读写
 */

#include "utils/ring/DataRingBuffer.h"

DataRingBuffer::DataRingBuffer(int capacity, QObject* parent)
    : QObject(parent), m_capacity(qMax(64, capacity))
{
    setObjectName(QStringLiteral("DataRingBuffer"));
    m_buffer.resize(m_capacity);
}

DataRingBuffer::~DataRingBuffer() = default;

void DataRingBuffer::setCapacity(int bytes)
{
    QWriteLocker locker(&m_lock);
    m_capacity = qMax(64, bytes);
    m_buffer.resize(m_capacity);
    m_head = m_tail = m_count = 0;
}

int DataRingBuffer::capacity() const { QReadLocker l(&m_lock); return m_capacity; }

void DataRingBuffer::setOverflowPolicy(OverflowPolicy p) { m_policy = p; }
DataRingBuffer::OverflowPolicy DataRingBuffer::overflowPolicy() const { return m_policy; }

int DataRingBuffer::write(const QByteArray& data)
{
    if (data.isEmpty()) return 0;
    QWriteLocker locker(&m_lock);

    int toWrite = qMin(data.size(), m_capacity);
    int dropped = 0;

    if (m_count + toWrite > m_capacity) {
        switch (m_policy) {
        case OverflowPolicy::OverwriteOldest: {
            int overflow = (m_count + toWrite) - m_capacity;
            m_tail = (m_tail + overflow) % m_capacity;
            m_count = qMax(0, m_count - overflow);
            dropped = overflow;
            break;
        }
        case OverflowPolicy::DropNewest:
            toWrite = qMax(0, m_capacity - m_count);
            if (toWrite == 0) {
                ++m_stats.totalOverflows;
                emit overflow(data.size());
                return 0;
            }
            break;
        case OverflowPolicy::ExpandCapacity:
            m_capacity = qMin(m_capacity * 2, 16777216);
            m_buffer.resize(m_capacity);
            dropped = 0;
            break;
        }
    }

    /* 写入数据到环形缓冲区 */
    for (int i = 0; i < toWrite; ++i) {
        m_buffer[m_head] = data[i];
        m_head = (m_head + 1) % m_capacity;
    }
    m_count += toWrite;
    if (m_count > m_capacity) m_count = m_capacity;

    ++m_stats.totalWrites;
    m_stats.totalBytesWritten += static_cast<quint64>(toWrite);
    m_sumWriteSize += static_cast<quint64>(toWrite);
    m_stats.avgWriteSize = static_cast<double>(m_sumWriteSize) / static_cast<double>(m_stats.totalWrites);

    if (dropped > 0) {
        m_stats.totalOverflows += static_cast<quint64>(dropped);
        emit overflow(dropped);
    }

    updateUtilization();
    emit dataWritten(toWrite);
    return toWrite;
}

QByteArray DataRingBuffer::read(int maxBytes)
{
    QWriteLocker locker(&m_lock);
    int toRead = qMin(qMax(0, maxBytes), m_count);
    if (toRead == 0) { ++m_stats.totalReadUnderruns; return {}; }

    QByteArray result;
    result.reserve(toRead);
    for (int i = 0; i < toRead; ++i) {
        result.append(m_buffer[m_tail]);
        m_tail = (m_tail + 1) % m_capacity;
    }
    m_count -= toRead;

    ++m_stats.totalReads;
    m_stats.totalBytesRead += static_cast<quint64>(toRead);
    m_sumReadSize += static_cast<quint64>(toRead);
    m_stats.avgReadSize = static_cast<double>(m_sumReadSize) / static_cast<double>(m_stats.totalReads);

    updateUtilization();
    emit dataRead(toRead);
    if (m_count == 0) emit bufferEmpty();
    return result;
}

QByteArray DataRingBuffer::peek(int maxBytes) const
{
    QReadLocker locker(&m_lock);
    int toRead = qMin(qMax(0, maxBytes), m_count);
    QByteArray result;
    result.reserve(toRead);
    int idx = m_tail;
    for (int i = 0; i < toRead; ++i) {
        result.append(m_buffer[idx]);
        idx = (idx + 1) % m_capacity;
    }
    return result;
}

int DataRingBuffer::skip(int bytes)
{
    QWriteLocker locker(&m_lock);
    int toSkip = qMin(bytes, m_count);
    m_tail = (m_tail + toSkip) % m_capacity;
    m_count -= toSkip;
    updateUtilization();
    return toSkip;
}

int DataRingBuffer::available() const { QReadLocker l(&m_lock); return m_count; }
int DataRingBuffer::freeSpace() const { QReadLocker l(&m_lock); return m_capacity - m_count; }
bool DataRingBuffer::isEmpty() const { QReadLocker l(&m_lock); return m_count == 0; }
bool DataRingBuffer::isFull() const { QReadLocker l(&m_lock); return m_count >= m_capacity; }

void DataRingBuffer::clear()
{
    QWriteLocker locker(&m_lock);
    m_head = m_tail = m_count = 0;
    updateUtilization();
}

DataRingBuffer::Stats DataRingBuffer::stats() const { return m_stats; }

void DataRingBuffer::resetStatistics() { m_stats = Stats{}; m_sumWriteSize = m_sumReadSize = 0; }

void DataRingBuffer::updateUtilization()
{
    m_stats.utilization = (m_capacity > 0)
        ? static_cast<double>(m_count) / static_cast<double>(m_capacity) : 0.0;
}
