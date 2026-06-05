/**
 * @file RingBufferEx.cpp
 * @brief 扩展环形缓冲区实现
 */

#include "utils/ringbuf2/RingBufferEx.h"

#include <QtMath>
#include <algorithm>
#include <numeric>

RingBufferEx::RingBufferEx(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(qMax(1, capacity))
    , m_head(0)
    , m_tail(0)
    , m_count(0)
{
    m_buffer.resize(m_capacity, 0.0);
}

void RingBufferEx::write(double value)
{
    bool overwritten = isFull();

    m_buffer[m_head] = value;
    m_head = (m_head + 1) % m_capacity;

    if (overwritten) {
        /* 满时覆盖，尾指针前移 */
        m_tail = (m_tail + 1) % m_capacity;
        ++m_stats.totalOverwrites;
        emit overflowOccurred(m_stats.totalOverwrites);
    } else {
        ++m_count;
    }

    ++m_stats.totalWrites;

    /* 更新峰值 */
    quint64 usage = static_cast<quint64>(m_count);
    if (usage > m_stats.peakUsage) {
        m_stats.peakUsage = usage;
    }

    emit dataWritten(m_count);
}

void RingBufferEx::writeBatch(const QVector<double>& values)
{
    for (double v : values) {
        write(v);
    }
}

QPair<bool, double> RingBufferEx::read()
{
    if (isEmpty()) {
        return {false, 0.0};
    }

    double value = m_buffer[m_tail];
    m_tail = (m_tail + 1) % m_capacity;
    --m_count;
    ++m_stats.totalReads;

    return {true, value};
}

QVector<double> RingBufferEx::readBatch(int maxCount)
{
    QVector<double> result;
    int toRead = qMin(maxCount, m_count);
    result.reserve(toRead);

    for (int i = 0; i < toRead; ++i) {
        auto [ok, val] = read();
        if (!ok) break;
        result.append(val);
    }

    return result;
}

QPair<bool, double> RingBufferEx::peekLatest() const
{
    if (isEmpty()) {
        return {false, 0.0};
    }
    /* 最新写入的位置是 head-1 */
    int latestIdx = (m_head - 1 + m_capacity) % m_capacity;
    return {true, m_buffer[latestIdx]};
}

RingBufferEx::Snapshot RingBufferEx::snapshot() const
{
    Snapshot snap;
    snap.count = static_cast<quint64>(m_count);

    if (m_count == 0) {
        snap.mean = 0.0;
        snap.stddev = 0.0;
        snap.min = 0.0;
        snap.max = 0.0;
        return snap;
    }

    /* 遍历有效元素(从tail到head) */
    double sum = 0.0;
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    for (int i = 0; i < m_count; ++i) {
        int idx = (m_tail + i) % m_capacity;
        double val = m_buffer[idx];
        sum += val;
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    snap.mean = sum / m_count;
    snap.min = minVal;
    snap.max = maxVal;

    /* 二次遍历计算标准差 */
    double sumSqDiff = 0.0;
    for (int i = 0; i < m_count; ++i) {
        int idx = (m_tail + i) % m_capacity;
        double diff = m_buffer[idx] - snap.mean;
        sumSqDiff += diff * diff;
    }

    snap.stddev = (m_count > 1) ? qSqrt(sumSqDiff / m_count) : 0.0;

    return snap;
}

int RingBufferEx::size() const
{
    return m_count;
}

bool RingBufferEx::isEmpty() const
{
    return m_count == 0;
}

bool RingBufferEx::isFull() const
{
    return m_count >= m_capacity;
}

int RingBufferEx::capacity() const
{
    return m_capacity;
}

void RingBufferEx::clear()
{
    m_head = 0;
    m_tail = 0;
    m_count = 0;
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
}

void RingBufferEx::resetStatistics()
{
    m_stats = Stats{};
}
