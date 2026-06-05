/**
 * @file BipBuffer.cpp
 * @brief 双端缓冲区实现
 */

#include "BipBuffer.h"
#include <QElapsedTimer>
#include <cstring>

BipBuffer::BipBuffer(int capacity, QObject* parent)
    : QObject(parent)
    , m_aStart(0), m_aEnd(0)
    , m_bStart(0), m_bEnd(0)
    , m_writeRegion(0)
    , m_timeSum(0.0)
{
    m_buffer.resize(qMax(64, capacity));
}

QPair<char*, int> BipBuffer::reserveWrite(int size)
{
    int cap = m_buffer.size();
    int requested = qMin(size, cap);

    /* 尝试A区 */
    if (m_writeRegion == 0 || m_writeRegion == 1) {
        int availableA = (m_bStart > m_aEnd) ? (m_bStart - m_aEnd) : (cap - m_aEnd);
        if (m_bEnd > 0 && m_bStart <= m_aEnd)
            availableA = cap - m_aEnd;

        if (availableA >= requested) {
            m_writeRegion = 1;
            return {m_buffer.data() + m_aEnd, requested};
        }
    }

    /* 尝试B区(从头开始) */
    if (m_aStart >= requested) {
        m_writeRegion = 2;
        return {m_buffer.data(), requested};
    }

    /* 无法满足 */
    m_writeRegion = 0;
    return {nullptr, 0};
}

void BipBuffer::commitWrite(int size)
{
    if (size <= 0) return;

    if (m_writeRegion == 1) {
        m_aEnd += size;
    } else if (m_writeRegion == 2) {
        m_bEnd = size;
    }
    m_writeRegion = 0;

    m_stats.totalWrites++;
    m_stats.totalBytesWritten += size;
}

QPair<const char*, int> BipBuffer::availableRead() const
{
    if (m_aStart < m_aEnd) {
        return {m_buffer.constData() + m_aStart, m_aEnd - m_aStart};
    }
    return {nullptr, 0};
}

void BipBuffer::commitRead(int size)
{
    if (size <= 0) return;

    m_aStart += size;
    m_stats.totalReads++;
    m_stats.totalBytesRead += size;

    /* A区读完, 切换到B区 */
    if (m_aStart >= m_aEnd) {
        if (m_bEnd > 0) {
            m_aStart = 0;
            m_aEnd = m_bEnd;
            m_bStart = 0;
            m_bEnd = 0;
        } else {
            m_aStart = 0;
            m_aEnd = 0;
        }
    }
}

int BipBuffer::write(const QByteArray& data)
{
    int size = qMin(data.size(), freeSpace());
    if (size <= 0) return 0;

    auto [ptr, avail] = reserveWrite(size);
    if (!ptr) return 0;

    int actual = qMin(size, avail);
    std::memcpy(ptr, data.constData(), actual);
    commitWrite(actual);
    return actual;
}

QByteArray BipBuffer::read(int maxSize)
{
    auto [ptr, avail] = availableRead();
    if (!ptr || avail == 0) return {};

    int size = qMin(maxSize, avail);
    QByteArray result(ptr, size);
    commitRead(size);
    return result;
}

int BipBuffer::capacity() const { return m_buffer.size(); }

int BipBuffer::available() const
{
    int a = m_aEnd - m_aStart;
    int b = m_bEnd - m_bStart;
    return qMax(0, a) + qMax(0, b);
}

int BipBuffer::freeSpace() const
{
    return m_buffer.size() - available();
}

bool BipBuffer::isEmpty() const { return available() == 0; }

void BipBuffer::clear()
{
    m_aStart = m_aEnd = 0;
    m_bStart = m_bEnd = 0;
    m_writeRegion = 0;
}

BipBuffer::Stats BipBuffer::stats() const { return m_stats; }

void BipBuffer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
