/**
 * @file CuckooFilter.cpp
 * @brief 布谷鸟过滤器实现
 */

#include "utils/cuckoo/CuckooFilter.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtGlobal>
#include <cstdlib>

CuckooFilter::CuckooFilter(int capacity, QObject* parent)
    : QObject(parent), m_capacity(qMax(16, capacity)),
      m_size(0), m_timeSum(0.0)
{
    m_numBuckets = (m_capacity + BUCKET_SIZE - 1) / BUCKET_SIZE;
    m_buckets.resize(m_numBuckets);
}

bool CuckooFilter::insert(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    quint8 fp = fingerprint(item);
    quint32 i1 = hashIndex(item) % m_numBuckets;
    quint32 i2 = altIndex(i1, fp) % m_numBuckets;

    /* 尝试插入到两个候选桶 */
    for (auto idx : {i1, i2}) {
        for (int j = 0; j < BUCKET_SIZE; ++j) {
            if (!m_buckets[idx].occupied[j]) {
                m_buckets[idx].fps[j] = fp;
                m_buckets[idx].occupied[j] = true;
                m_size++;
                m_stats.totalInsertions++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInsertions, 1ULL);
                emit elementInserted(item);
                return true;
            }
        }
    }

    /* 需要踢出 */
    quint32 idx = (QRandomGenerator::global()->bounded(1000000) % 2 == 0) ? i1 : i2;
    for (int kick = 0; kick < MAX_KICKS; ++kick) {
        int slot = QRandomGenerator::global()->bounded(1000000) % BUCKET_SIZE;
        quint8 evictedFp = m_buckets[idx].fps[slot];
        m_buckets[idx].fps[slot] = fp;
        fp = evictedFp;

        idx = altIndex(idx, fp) % m_numBuckets;
        for (int j = 0; j < BUCKET_SIZE; ++j) {
            if (!m_buckets[idx].occupied[j]) {
                m_buckets[idx].fps[j] = fp;
                m_buckets[idx].occupied[j] = true;
                m_size++;
                m_stats.totalInsertions++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInsertions, 1ULL);
                emit elementInserted(item);
                return true;
            }
        }
    }

    emit filterFull();
    return false;
}

bool CuckooFilter::contains(const QByteArray& item) const
{
    quint8 fp = fingerprint(item);
    quint32 i1 = hashIndex(item) % m_numBuckets;
    quint32 i2 = altIndex(i1, fp) % m_numBuckets;

    for (auto idx : {i1, i2}) {
        for (int j = 0; j < BUCKET_SIZE; ++j) {
            if (m_buckets[idx].occupied[j] && m_buckets[idx].fps[j] == fp) {
                const_cast<CuckooFilter*>(this)->m_stats.totalQueries++;
                return true;
            }
        }
    }

    const_cast<CuckooFilter*>(this)->m_stats.totalQueries++;
    return false;
}

bool CuckooFilter::remove(const QByteArray& item)
{
    quint8 fp = fingerprint(item);
    quint32 i1 = hashIndex(item) % m_numBuckets;
    quint32 i2 = altIndex(i1, fp) % m_numBuckets;

    for (auto idx : {i1, i2}) {
        for (int j = 0; j < BUCKET_SIZE; ++j) {
            if (m_buckets[idx].occupied[j] && m_buckets[idx].fps[j] == fp) {
                m_buckets[idx].occupied[j] = false;
                m_size--;
                m_stats.totalDeletions++;
                emit elementRemoved(item);
                return true;
            }
        }
    }
    return false;
}

double CuckooFilter::fillRatio() const
{
    return (m_numBuckets * BUCKET_SIZE > 0)
        ? static_cast<double>(m_size) / (m_numBuckets * BUCKET_SIZE) : 0.0;
}

void CuckooFilter::clear()
{
    for (auto& b : m_buckets) {
        for (int j = 0; j < BUCKET_SIZE; ++j) b.occupied[j] = false;
    }
    m_size = 0;
}

quint8 CuckooFilter::fingerprint(const QByteArray& item) const
{
    quint32 h = 0;
    for (int i = 0; i < item.size(); ++i) {
        h = h * 31 + static_cast<quint8>(item[i]);
    }
    return static_cast<quint8>((h & 0xFF) | 1); /* 确保非零 */
}

quint32 CuckooFilter::hashIndex(const QByteArray& item) const
{
    quint32 h = 0;
    for (int i = 0; i < item.size(); ++i) {
        h ^= static_cast<quint8>(item[i]);
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

quint32 CuckooFilter::altIndex(quint32 idx, quint8 fp) const
{
    return idx ^ (fp * 0x5bd1e995);
}

void CuckooFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
