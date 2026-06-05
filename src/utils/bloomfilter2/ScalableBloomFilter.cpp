/**
 * @file ScalableBloomFilter.cpp
 * @brief 可扩展布隆过滤器实现
 */

#include "utils/bloomfilter2/ScalableBloomFilter.h"

#include <QElapsedTimer>
#include <QtMath>

ScalableBloomFilter::ScalableBloomFilter(double fpRate, QObject* parent)
    : QObject(parent), m_fpRate(qBound(0.001, fpRate, 0.5)),
      m_totalElements(0), m_scaleFactor(1000), m_growthRate(2.0),
      m_timeSum(0.0)
{
    addLayer();
}

void ScalableBloomFilter::insert(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    /* 检查当前层是否快满 */
    if (!m_filters.isEmpty()) {
        auto& last = m_filters.last();
        if (last.inserted >= last.capacity) {
            addLayer();
        }
    }

    /* 插入到最新层 */
    auto& layer = m_filters.last();
    int h1 = qHash(item) & 0x7FFFFFFF;
    int h2 = qHash(item + QByteArray(1, '~')) & 0x7FFFFFFF;

    for (int i = 0; i < layer.hashCount; ++i) {
        int pos = (h1 + i * h2) % layer.bitCount;
        layer.bits[pos / 64] |= (1ULL << (pos % 64));
    }
    layer.inserted++;
    m_totalElements++;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions, 1ULL);

    emit elementInserted(m_totalElements);
}

bool ScalableBloomFilter::contains(const QByteArray& item) const
{
    int h1 = qHash(item) & 0x7FFFFFFF;
    int h2 = qHash(item + QByteArray(1, '~')) & 0x7FFFFFFF;

    for (const auto& layer : m_filters) {
        bool found = true;
        for (int i = 0; i < layer.hashCount; ++i) {
            int pos = (h1 + i * h2) % layer.bitCount;
            if (!(layer.bits[pos / 64] & (1ULL << (pos % 64)))) {
                found = false;
                break;
            }
        }
        if (found) {
            m_stats.totalQueries++;
            return true;
        }
    }

    m_stats.totalQueries++;
    return false;
}

void ScalableBloomFilter::addLayer()
{
    BloomLayer layer;
    layer.capacity = (m_filters.isEmpty()) ? m_scaleFactor
        : static_cast<quint64>(m_filters.last().capacity * m_growthRate);

    /* 每层的FP率收紧 */
    double layerFp = m_fpRate / qPow(2.0, m_filters.size() + 1);
    double ln2 = qLn(2.0);
    layer.bitCount = qMax(64, static_cast<int>(
        qCeil(-static_cast<double>(layer.capacity) * qLn(layerFp) /
              (ln2 * ln2))));
    layer.hashCount = qMax(1, static_cast<int>(
        qRound(static_cast<double>(layer.bitCount) / layer.capacity * ln2)));

    int wordCount = (layer.bitCount + 63) / 64;
    layer.bits.resize(wordCount, 0);
    layer.inserted = 0;

    m_filters.append(layer);
    m_stats.totalLayers = m_filters.size();

    emit layerAdded(m_filters.size() - 1);
}

void ScalableBloomFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
