/**
 * @file MinHashSignature.cpp
 * @brief MinHash签名实现
 */

#include "utils/minhash/MinHashSignature.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

MinHashSignature::MinHashSignature(int numHashes, QObject* parent)
    : QObject(parent), m_numHashes(qMax(8, numHashes)), m_timeSum(0.0) {}

QVector<quint32> MinHashSignature::computeSignature(
    const QSet<QByteArray>& items)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint32> signature(m_numHashes, 0xFFFFFFFF);

    for (const auto& item : items) {
        for (int h = 0; h < m_numHashes; ++h) {
            quint32 hash = hashItem(item, h);
            if (hash < signature[h]) {
                signature[h] = hash;
            }
        }
    }

    m_stats.totalHashes += items.size() * m_numHashes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalHashes + m_stats.totalComparisons, 1ULL);

    emit signatureComputed(items.size());
    return signature;
}

double MinHashSignature::jaccardSimilarity(
    const QVector<quint32>& sig1, const QVector<quint32>& sig2) const
{
    if (sig1.size() != sig2.size() || sig1.isEmpty()) return 0.0;

    int matches = 0;
    for (int i = 0; i < sig1.size(); ++i) {
        if (sig1[i] == sig2[i]) ++matches;
    }
    return static_cast<double>(matches) / sig1.size();
}

QVector<QVector<quint32>> MinHashSignature::batchSignatures(
    const QVector<QSet<QByteArray>>& itemSets)
{
    QVector<QVector<quint32>> result;
    result.reserve(itemSets.size());
    for (const auto& set : itemSets) {
        result.append(computeSignature(set));
    }
    return result;
}

QVector<QPair<int, double>> MinHashSignature::topKSimilar(
    const QVector<quint32>& query,
    const QVector<QVector<quint32>>& candidates, int k) const
{
    QVector<QPair<int, double>> scored;
    scored.reserve(candidates.size());
    for (int i = 0; i < candidates.size(); ++i) {
        double sim = jaccardSimilarity(query, candidates[i]);
        scored.append({i, sim});
    }

    std::partial_sort(scored.begin(), scored.begin() + qMin(k, scored.size()),
                      scored.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });

    scored.resize(qMin(k, scored.size()));
    return scored;
}

quint32 MinHashSignature::hashItem(const QByteArray& item, int hashIdx) const
{
    return murmurHash(item, static_cast<quint32>(hashIdx * 0x9e3779b9));
}

quint32 MinHashSignature::murmurHash(const QByteArray& data,
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

void MinHashSignature::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
