/**
 * @file StableBloomFilter.cpp
 * @brief Stable Bloom Filter实现
 */

#include "StableBloomFilter.h"
#include <QElapsedTimer>
#include <QRandomGenerator>

StableBloomFilter::StableBloomFilter(int numCells, int numHashes,
                                       int maxCounter, double decayRate,
                                       QObject* parent)
    : QObject(parent)
    , m_cells(numCells, 0)
    , m_numCells(qMax(64, numCells))
    , m_numHashes(qMax(1, numHashes))
    , m_maxCounter(qMax(1, maxCounter))
    , m_decayRate(qBound(0.0, decayRate, 1.0))
    , m_timeSum(0.0)
{
}

quint64 StableBloomFilter::hash(const QByteArray& data, int seed) const
{
    quint64 h = static_cast<quint64>(seed) * 0x9E3779B97F4A7C15ULL;
    for (char c : data) {
        h ^= static_cast<quint64>(static_cast<quint8>(c));
        h *= 0xBF58476D1CE4E5B9ULL;
        h ^= h >> 31;
    }
    return h;
}

void StableBloomFilter::add(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    decay();

    for (int i = 0; i < m_numHashes; ++i) {
        quint64 idx = hash(data, i) % static_cast<quint64>(m_numCells);
        m_cells[static_cast<int>(idx)] = static_cast<quint8>(m_maxCounter);
    }

    m_stats.totalAdds++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalAdds + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit elementAdded(data);
}

bool StableBloomFilter::contains(const QByteArray& data) const
{
    QElapsedTimer timer;
    timer.start();

    bool found = true;
    for (int i = 0; i < m_numHashes; ++i) {
        quint64 idx = hash(data, i) % static_cast<quint64>(m_numCells);
        if (m_cells[static_cast<int>(idx)] == 0) {
            found = false;
            break;
        }
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalAdds + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return found;
}

void StableBloomFilter::decay()
{
    for (int i = 0; i < m_numCells; ++i) {
        if (m_cells[i] > 0 && QRandomGenerator::global()->generateDouble() < m_decayRate) {
            m_cells[i]--;
        }
    }
}

void StableBloomFilter::addString(const QString& str)
{
    add(str.toUtf8());
}

bool StableBloomFilter::containsString(const QString& str) const
{
    return contains(str.toUtf8());
}

void StableBloomFilter::clear()
{
    m_cells.fill(0);
}

double StableBloomFilter::estimatedFPR() const
{
    int nonZero = 0;
    for (quint8 c : m_cells)
        if (c > 0) nonZero++;

    double ratio = static_cast<double>(nonZero) / m_numCells;
    return std::pow(ratio, m_numHashes);
}

int StableBloomFilter::cellCount() const { return m_numCells; }

StableBloomFilter::Stats StableBloomFilter::stats() const { return m_stats; }

void StableBloomFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
