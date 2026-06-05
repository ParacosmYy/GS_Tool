/**
 * @file BloomFilter.cpp
 * @brief 布隆过滤器实现 — MurmurHash3多哈希
 */

#include "utils/bloom/BloomFilter.h"

#include <QtMath>

BloomFilter::BloomFilter(quint64 expectedItems, double falsePositiveRate,
                         QObject* parent)
    : QObject(parent), m_insertedCount(0)
{
    /* 防止expectedItems为0导致除零 */
    expectedItems = qMax(expectedItems, static_cast<quint64>(1));

    /* 最优参数: m = -n*ln(p) / (ln2)^2, k = (m/n)*ln2 */
    double ln2 = qLn(2.0);
    m_bitCount = static_cast<quint64>(
        qCeil(-static_cast<double>(expectedItems) * qLn(falsePositiveRate)
              / (ln2 * ln2)));
    m_bitCount = qMax(m_bitCount, static_cast<quint64>(64));

    m_hashCount = qMax(1, static_cast<int>(
        qRound(static_cast<double>(m_bitCount) / expectedItems * ln2)));

    quint64 wordCount = (m_bitCount + 63) / 64;
    m_bits.resize(wordCount, 0);
}

void BloomFilter::insert(const QByteArray& data)
{
    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(data, i) % m_bitCount;
        m_bits[pos / 64] |= (1ULL << (pos % 64));
    }
    ++m_insertedCount;
    ++m_stats.totalInsertions;
    emit elementInserted(m_insertedCount);
}

bool BloomFilter::contains(const QByteArray& data) const
{
    ++m_stats.totalQueries;
    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(data, i) % m_bitCount;
        if (!(m_bits[pos / 64] & (1ULL << (pos % 64)))) {
            ++m_stats.totalTrueNegatives;
            return false;
        }
    }
    /* 可能是真阳性或假阳性 */
    return true;
}

void BloomFilter::clear()
{
    m_bits.fill(0);
    m_insertedCount = 0;
}

double BloomFilter::expectedFalsePositiveRate() const
{
    if (m_insertedCount == 0) return 0.0;
    double exponent = -static_cast<double>(m_hashCount) * m_insertedCount
                      / m_bitCount;
    return qPow(1.0 - qExp(exponent), m_hashCount);
}

double BloomFilter::fillRatio() const
{
    quint64 setBits = 0;
    for (quint64 word : m_bits) {
        setBits += qPopulationCount(word);
    }
    return (m_bitCount > 0) ? static_cast<double>(setBits) / m_bitCount : 0.0;
}

quint32 BloomFilter::hashN(const QByteArray& data, int n) const
{
    return murmurHash(data, static_cast<quint32>(n * 0x9e3779b9));
}

quint32 BloomFilter::murmurHash(const QByteArray& data, quint32 seed) const
{
    quint32 h = seed;
    int len = data.size();
    const char* ptr = data.constData();

    int i = 0;
    for (; i + 4 <= len; i += 4) {
        quint32 k = static_cast<quint32>(
            static_cast<quint8>(ptr[i]) |
            (static_cast<quint8>(ptr[i + 1]) << 8) |
            (static_cast<quint8>(ptr[i + 2]) << 16) |
            (static_cast<quint8>(ptr[i + 3]) << 24));
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

void BloomFilter::resetStatistics() { m_stats = Stats{}; }
