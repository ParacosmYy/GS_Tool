/**
 * @file CuckooFilter2.cpp
 * @brief Cuckoo Filter增强版实现
 */

#include "CuckooFilter2.h"
#include <QElapsedTimer>
#include <QRandomGenerator>

CuckooFilter2::CuckooFilter2(int capacity, int bucketSize, int fingerprintSize,
                               int maxKicks, QObject* parent)
    : QObject(parent)
    , m_bucketSize(qMax(2, bucketSize))
    , m_fpSize(qBound(4, fingerprintSize, 16))
    , m_maxKicks(maxKicks)
    , m_count(0)
    , m_timeSum(0.0)
{
    m_numBuckets = qMax(1, capacity / m_bucketSize);
    /* 确保桶数是2的幂 */
    int pow2 = 1;
    while (pow2 < m_numBuckets) pow2 *= 2;
    m_numBuckets = pow2;

    m_buckets.resize(m_numBuckets * m_bucketSize, 0);
}

quint32 CuckooFilter2::hash(const QByteArray& data) const
{
    quint64 h = 0xCBF29CE484222325ULL;
    for (char c : data) {
        h ^= static_cast<quint64>(static_cast<quint8>(c));
        h *= 0x100000001B3ULL;
    }
    return static_cast<quint32>(h);
}

quint32 CuckooFilter2::fingerprint(const QByteArray& data) const
{
    quint32 h = hash(data);
    quint32 mask = (1U << m_fpSize) - 1;
    quint32 fp = h & mask;
    return (fp == 0) ? 1 : fp;  /* 指纹不能为0 */
}

quint32 CuckooFilter2::altIndex(quint32 index, quint32 fp) const
{
    quint32 h = hash(QByteArray(reinterpret_cast<const char*>(&fp), sizeof(fp)));
    return (index ^ h) % static_cast<quint32>(m_numBuckets);
}

bool CuckooFilter2::insert(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    quint32 fp = fingerprint(item);
    quint32 i1 = hash(item) % static_cast<quint32>(m_numBuckets);
    quint32 i2 = altIndex(i1, fp);

    /* 尝试插入i1或i2 */
    for (int b : {static_cast<int>(i1), static_cast<int>(i2)}) {
        for (int j = 0; j < m_bucketSize; ++j) {
            int idx = b * m_bucketSize + j;
            if (m_buckets[idx] == 0) {
                m_buckets[idx] = fp;
                m_count++;

                m_stats.totalInserts++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalLookups);

                emit inserted(item, true);
                return true;
            }
        }
    }

    /* 踢出 */
    quint32 index = (QRandomGenerator::global()->bounded(2) == 0) ? i1 : i2;
    for (int kick = 0; kick < m_maxKicks; ++kick) {
        int slot = QRandomGenerator::global()->bounded(m_bucketSize);
        int idx = static_cast<int>(index) * m_bucketSize + slot;

        quint32 evictedFp = m_buckets[idx];
        m_buckets[idx] = fp;
        fp = evictedFp;

        index = altIndex(index, fp);
        for (int j = 0; j < m_bucketSize; ++j) {
            int tidx = static_cast<int>(index) * m_bucketSize + j;
            if (m_buckets[tidx] == 0) {
                m_buckets[tidx] = fp;
                m_count++;

                m_stats.totalInserts++;
                m_stats.totalKicks += kick + 1;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalLookups);

                emit inserted(item, true);
                return true;
            }
        }
    }

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalLookups);

    emit inserted(item, false);
    return false;
}

bool CuckooFilter2::contains(const QByteArray& item) const
{
    QElapsedTimer timer;
    timer.start();

    quint32 fp = fingerprint(item);
    quint32 i1 = hash(item) % static_cast<quint32>(m_numBuckets);
    quint32 i2 = altIndex(i1, fp);

    bool found = false;
    for (int b : {static_cast<int>(i1), static_cast<int>(i2)}) {
        for (int j = 0; j < m_bucketSize; ++j) {
            int idx = b * m_bucketSize + j;
            if (m_buckets[idx] == fp) { found = true; break; }
        }
        if (found) break;
    }

    m_stats.totalLookups++;
    if (found) m_stats.totalHits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalLookups);

    return found;
}

bool CuckooFilter2::remove(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    quint32 fp = fingerprint(item);
    quint32 i1 = hash(item) % static_cast<quint32>(m_numBuckets);
    quint32 i2 = altIndex(i1, fp);

    for (int b : {static_cast<int>(i1), static_cast<int>(i2)}) {
        for (int j = 0; j < m_bucketSize; ++j) {
            int idx = b * m_bucketSize + j;
            if (m_buckets[idx] == fp) {
                m_buckets[idx] = 0;
                m_count--;

                m_stats.totalDeletes++;
                m_timeSum += timer.elapsed();
                return true;
            }
        }
    }

    m_timeSum += timer.elapsed();
    return false;
}

bool CuckooFilter2::insertString(const QString& str) { return insert(str.toUtf8()); }
bool CuckooFilter2::containsString(const QString& str) const { return contains(str.toUtf8()); }
int CuckooFilter2::count() const { return m_count; }
int CuckooFilter2::capacity() const { return m_numBuckets * m_bucketSize; }
double CuckooFilter2::loadFactor() const { return static_cast<double>(m_count) / m_buckets.size(); }
CuckooFilter2::Stats CuckooFilter2::stats() const { return m_stats; }

void CuckooFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
