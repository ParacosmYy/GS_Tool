/**
 * @file ScalingBloomFilter.cpp
 * @brief 可扩展布隆过滤器实现
 */

#include "ScalingBloomFilter.h"
#include <QElapsedTimer>
#include <cmath>

ScalingBloomFilter::ScalingBloomFilter(double fpRate, int initialCapacity,
                                           QObject* parent)
    : QObject(parent)
    , m_targetFpRate(fpRate)
    , m_initialCapacity(initialCapacity)
    , m_totalInserts(0)
    , m_timeSum(0.0)
{
    /* 初始slice */
    int bitsPerItem = static_cast<int>(std::ceil(-std::log(fpRate) / (std::log(2) * std::log(2))));
    int hashCount = qMax(1, static_cast<int>(std::round(std::log(2.0) * bitsPerItem)));
    int totalBits = bitsPerItem * initialCapacity;

    Slice s;
    s.bits.resize(totalBits, false);
    s.hashCount = hashCount;
    s.capacity = initialCapacity;
    s.inserts = 0;
    m_slices.append(s);
}

void ScalingBloomFilter::insert(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    Slice& current = m_slices.last();
    current.inserts++;
    m_totalInserts++;

    for (int i = 0; i < current.hashCount; ++i) {
        quint64 h = hash(item, i);
        current.bits[static_cast<int>(h % current.bits.size())] = true;
    }

    /* 检查是否需要扩容 */
    if (current.inserts >= current.capacity) {
        scaleUp();
    }

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

void ScalingBloomFilter::insertString(const QString& item)
{
    insert(item.toUtf8());
}

bool ScalingBloomFilter::contains(const QByteArray& item) const
{
    for (const auto& slice : m_slices) {
        bool found = true;
        for (int i = 0; i < slice.hashCount; ++i) {
            quint64 h = hash(item, i);
            if (!slice.bits[static_cast<int>(h % slice.bits.size())]) {
                found = false;
                break;
            }
        }
        if (found) return true;
    }
    return false;
}

bool ScalingBloomFilter::containsString(const QString& item) const
{
    return contains(item.toUtf8());
}

long long ScalingBloomFilter::estimateCount() const { return m_totalInserts; }

double ScalingBloomFilter::estimatedFalsePositiveRate() const
{
    double rate = 1.0;
    for (const auto& slice : m_slices) {
        if (slice.bits.isEmpty() || slice.inserts == 0) continue;
        double fillRatio = static_cast<double>(slice.inserts * slice.hashCount) / slice.bits.size();
        fillRatio = qMin(1.0, fillRatio);
        double sliceFp = std::pow(fillRatio, slice.hashCount);
        rate *= (1.0 - sliceFp);
    }
    return 1.0 - rate;
}

int ScalingBloomFilter::capacity() const
{
    int total = 0;
    for (const auto& s : m_slices) total += s.capacity;
    return total;
}

int ScalingBloomFilter::scaleCount() const { return m_slices.size() - 1; }

void ScalingBloomFilter::scaleUp()
{
    int newCapacity = m_slices.last().capacity * 2;
    int bitsPerItem = static_cast<int>(std::ceil(-std::log(m_targetFpRate) / (std::log(2) * std::log(2))));
    int hashCount = qMax(1, static_cast<int>(std::round(std::log(2.0) * bitsPerItem)));

    Slice s;
    s.bits.resize(bitsPerItem * newCapacity, false);
    s.hashCount = hashCount;
    s.capacity = newCapacity;
    s.inserts = 0;
    m_slices.append(s);

    emit scaled(s.bits.size());
}

quint64 ScalingBloomFilter::hash(const QByteArray& data, int seed) const
{
    quint64 h = static_cast<quint64>(seed) * 0x9E3779B97F4A7C15LL;
    for (char c : data) {
        h ^= static_cast<quint64>(static_cast<quint8>(c));
        h *= 0xBF58476D1CE4E5B9LL;
        h ^= h >> 31;
    }
    return h;
}

ScalingBloomFilter::Stats ScalingBloomFilter::stats() const { return m_stats; }

void ScalingBloomFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
