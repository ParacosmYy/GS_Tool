/**
 * @file CountingBloomFilter.cpp
 * @brief 计数型布隆过滤器实现 — 支持删除的MurmurHash3多哈希
 */

#include "CountingBloomFilter.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---------- 构造函数 ---------- */

CountingBloomFilter::CountingBloomFilter(quint64 expectedItems,
                                          double falsePositiveRate,
                                          int counterBits,
                                          QObject* parent)
    : QObject(parent), m_counterBits(counterBits), m_itemCount(0)
{
    /* 参数安全检查 */
    expectedItems = qMax(expectedItems, static_cast<quint64>(1));
    falsePositiveRate = qBound(1e-10, falsePositiveRate, 0.5);
    m_counterBits = qBound(1, m_counterBits, 8);

    /* 最优参数计算(同标准布隆过滤器) */
    double ln2 = qLn(2.0);
    m_slotCount = static_cast<quint64>(
        qCeil(-static_cast<double>(expectedItems) * qLn(falsePositiveRate)
              / (ln2 * ln2)));
    m_slotCount = qMax(m_slotCount, static_cast<quint64>(64));

    m_hashCount = qMax(1, static_cast<int>(
        qRound(static_cast<double>(m_slotCount) / expectedItems * ln2)));

    /* 计数器存储: 将counterBits位计数器打包到字节中 */
    int countersPerByte = 8 / m_counterBits;
    quint64 totalBytes = (m_slotCount + countersPerByte - 1) / countersPerByte;
    m_counters.resize(static_cast<int>(totalBytes), 0);
}

/* ---------- 插入 ---------- */

void CountingBloomFilter::insert(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(data, i) % m_slotCount;
        if (!incrementCounter(pos)) {
            emit counterOverflow(pos);
        }
    }
    ++m_itemCount;
    ++m_stats.totalInsertions;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0)
        ? m_timeSum / m_stats.totalInsertions : 0.0;

    emit elementInserted(m_itemCount);
}

/* ---------- 删除 ---------- */

bool CountingBloomFilter::remove(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 先检查所有计数器是否>0 */
    QVector<quint32> positions;
    positions.reserve(m_hashCount);
    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(data, i) % m_slotCount;
        positions.append(pos);
        if (getCounter(pos) == 0) {
            /* 元素不存在，无法删除 */
            return false;
        }
    }

    /* 所有位置计数器>0，执行递减 */
    for (quint32 pos : positions) {
        decrementCounter(pos);
    }

    --m_itemCount;
    ++m_stats.totalRemovals;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalRemovals > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalRemovals) : 0.0;

    emit elementRemoved(m_itemCount);
    return true;
}

/* ---------- 查询 ---------- */

bool CountingBloomFilter::contains(const QByteArray& data) const
{
    QElapsedTimer timer;
    timer.start();

    ++m_stats.totalQueries;

    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(data, i) % m_slotCount;
        if (getCounter(pos) == 0) {
            /* 一定不存在 */
            m_timeSum += timer.elapsed();
            return false;
        }
    }

    /* 可能是真阳性或假阳性 */
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalQueries > 0)
        ? m_timeSum / m_stats.totalQueries : 0.0;

    return true;
}

/* ---------- 清空 ---------- */

void CountingBloomFilter::clear()
{
    m_counters.fill(0);
    m_itemCount = 0;
}

/* ---------- 估计误判率 ---------- */

double CountingBloomFilter::estimatedFalsePositiveRate() const
{
    if (m_itemCount == 0 || m_slotCount == 0) return 0.0;
    double exponent = -static_cast<double>(m_hashCount) * m_itemCount
                      / m_slotCount;
    return qPow(1.0 - qExp(exponent), m_hashCount);
}

/* ---------- 统计 ---------- */

CountingBloomFilter::Stats CountingBloomFilter::stats() const { return m_stats; }

void CountingBloomFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- MurmurHash3 ---------- */

quint32 CountingBloomFilter::murmurHash(const QByteArray& data,
                                         quint32 seed) const
{
    quint32 h = seed;
    int len = data.size();
    const char* ptr = data.constData();

    int i = 0;
    for (; i + 4 <= len; i += 4) {
        quint32 k = static_cast<quint32>(
            static_cast<quint8>(ptr[i]) |
            (static_cast<quint32>(static_cast<quint8>(ptr[i + 1])) << 8) |
            (static_cast<quint32>(static_cast<quint8>(ptr[i + 2])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(ptr[i + 3])) << 24));
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    quint32 remaining = 0;
    int shift = 0;
    for (; i < len; ++i) {
        remaining |= static_cast<quint32>(static_cast<quint8>(ptr[i])) << shift;
        shift += 8;
    }
    if (shift > 0) {
        remaining *= 0xcc9e2d51;
        remaining = (remaining << 15) | (remaining >> 17);
        remaining *= 0x1b873593;
        h ^= remaining;
    }

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

quint32 CountingBloomFilter::hashN(const QByteArray& data, int n) const
{
    return murmurHash(data, static_cast<quint32>(n * 0x9e3779b9));
}

/* ---------- 计数器读写 ---------- */

quint8 CountingBloomFilter::getCounter(quint64 position) const
{
    int countersPerByte = 8 / m_counterBits;
    quint64 byteIdx = position / countersPerByte;
    int bitOffset = static_cast<int>((position % countersPerByte) * m_counterBits);
    quint8 mask = static_cast<quint8>((1 << m_counterBits) - 1);

    if (byteIdx >= static_cast<quint64>(m_counters.size())) return 0;
    return (m_counters[static_cast<int>(byteIdx)] >> bitOffset) & mask;
}

void CountingBloomFilter::setCounter(quint64 position, quint8 value)
{
    int countersPerByte = 8 / m_counterBits;
    quint64 byteIdx = position / countersPerByte;
    int bitOffset = static_cast<int>((position % countersPerByte) * m_counterBits);
    quint8 mask = static_cast<quint8>((1 << m_counterBits) - 1);

    if (byteIdx >= static_cast<quint64>(m_counters.size())) return;

    /* 清除旧值并设置新值 */
    int idx = static_cast<int>(byteIdx);
    m_counters[idx] &= ~(mask << bitOffset);
    m_counters[idx] |= ((value & mask) << bitOffset);
}

bool CountingBloomFilter::incrementCounter(quint64 position)
{
    quint8 current = getCounter(position);
    quint8 maxVal = static_cast<quint8>((1 << m_counterBits) - 1);
    if (current >= maxVal) {
        /* 计数器溢出，保持最大值 */
        return false;
    }
    setCounter(position, current + 1);
    return true;
}

bool CountingBloomFilter::decrementCounter(quint64 position)
{
    quint8 current = getCounter(position);
    if (current == 0) {
        return false;
    }
    setCounter(position, current - 1);
    return true;
}
