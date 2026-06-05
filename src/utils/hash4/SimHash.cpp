/**
 * @file SimHash.cpp
 * @brief SimHash局部敏感哈希实现
 */

#include "SimHash.h"
#include <QElapsedTimer>

SimHash::SimHash(int hashBits, QObject* parent)
    : QObject(parent)
    , m_hashBits(qBound(8, hashBits, 64))
    , m_timeSum(0.0)
{
}

quint64 SimHash::hashFeature(const QString& feature) const
{
    QByteArray data = feature.toUtf8();
    quint64 h = 0xCBF29CE484222325ULL;
    for (char c : data) {
        h ^= static_cast<quint64>(static_cast<quint8>(c));
        h *= 0x100000001B3ULL;
    }
    return h;
}

quint64 SimHash::compute(const QVector<QPair<QString, double>>& features) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> v(m_hashBits, 0.0);

    for (const auto& [feature, weight] : features) {
        quint64 h = hashFeature(feature);
        for (int i = 0; i < m_hashBits; ++i) {
            if ((h >> i) & 1)
                v[i] += weight;
            else
                v[i] -= weight;
        }
    }

    quint64 fingerprint = 0;
    for (int i = 0; i < m_hashBits; ++i) {
        if (v[i] > 0)
            fingerprint |= (1ULL << i);
    }

    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalComputed + m_stats.totalComparisons;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit hashComputed(fingerprint);
    return fingerprint;
}

quint64 SimHash::fromText(const QString& text, int ngramSize) const
{
    QVector<QPair<QString, double>> features;
    int n = text.size();

    for (int i = 0; i <= n - ngramSize; ++i) {
        QString ngram = text.mid(i, ngramSize).toLower();
        bool found = false;
        for (auto& f : features) {
            if (f.first == ngram) { f.second += 1.0; found = true; break; }
        }
        if (!found) features.append({ngram, 1.0});
    }

    return compute(features);
}

int SimHash::hammingDistance(quint64 h1, quint64 h2) const
{
    quint64 xorVal = h1 ^ h2;
    int dist = 0;
    while (xorVal) {
        dist += xorVal & 1;
        xorVal >>= 1;
    }
    return dist;
}

bool SimHash::isSimilar(quint64 h1, quint64 h2, int threshold) const
{
    m_stats.totalComparisons++;
    bool similar = hammingDistance(h1, h2) <= threshold;
    if (similar) m_stats.totalSimilar++;
    return similar;
}

double SimHash::similarity(quint64 h1, quint64 h2) const
{
    int dist = hammingDistance(h1, h2);
    return 1.0 - static_cast<double>(dist) / m_hashBits;
}

SimHash::Stats SimHash::stats() const { return m_stats; }

void SimHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
