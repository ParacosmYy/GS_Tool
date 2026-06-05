/**
 * @file ScalableBloomFilter.cpp
 * @brief 可扩展布隆过滤器实现
 */

#include "utils/bloomfilter3/ScalableBloomFilter.h"

#include <QElapsedTimer>
#include <QtMath>

ScalableBloomFilter3::ScalableBloomFilter3(double targetFpRate, QObject* parent)
    : QObject(parent), m_targetFpRate(qBound(0.001, targetFpRate, 0.5)),
      m_totalElements(0), m_initialCapacity(1000),
      m_growthFactor(2.0), m_tighteningRatio(0.85), m_timeSum(0.0)
{
    addLayer();
}

void ScalableBloomFilter3::insert(const QByteArray& item)
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

    /* 插入到所有层(提高查询准确率) */
    auto& layer = m_filters.last();
    for (int i = 0; i < layer.hashCount; ++i) {
        int pos = hashAt(item, i, layer.bitCount);
        layer.bits[pos / 64] |= (1ULL << (pos % 64));
    }
    layer.inserted++;
    m_totalElements++;

    m_stats.totalInsertions++;
    m_stats.totalLayers = m_filters.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions + m_stats.totalQueries, 1ULL);

    emit elementInserted(m_totalElements);
}

bool ScalableBloomFilter3::contains(const QByteArray& item)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalQueries++;

    /* 逐层检查 */
    for (const auto& layer : m_filters) {
        bool found = true;
        for (int i = 0; i < layer.hashCount; ++i) {
            int pos = hashAt(item, i, layer.bitCount);
            if (!(layer.bits[pos / 64] & (1ULL << (pos % 64)))) {
                found = false;
                break;
            }
        }
        if (found) {
            m_stats.totalFalsePositiveChecks++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInsertions + m_stats.totalQueries, 1ULL);
            return true;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInsertions + m_stats.totalQueries, 1ULL);
    return false;
}

double ScalableBloomFilter3::estimateFalsePositiveRate() const
{
    double totalFp = 1.0;
    for (int i = 0; i < m_filters.size(); ++i) {
        const auto& layer = m_filters[i];
        double layerFp = m_targetFpRate * qPow(m_tighteningRatio, i);
        totalFp *= (1.0 - layerFp);
    }
    return 1.0 - totalFp;
}

void ScalableBloomFilter3::clear()
{
    m_filters.clear();
    m_totalElements = 0;
    addLayer();
}

void ScalableBloomFilter3::addLayer()
{
    BloomLayer layer;
    layer.capacity = (m_filters.isEmpty())
        ? static_cast<quint64>(m_initialCapacity)
        : static_cast<quint64>(m_filters.last().capacity * m_growthFactor);

    /* 每层FP率收紧 */
    double layerFp = m_targetFpRate * qPow(m_tighteningRatio, m_filters.size());
    double ln2 = qLn(2.0);
    layer.bitCount = qMax(64, static_cast<int>(
        qCeil(-static_cast<double>(layer.capacity) * qLn(layerFp) /
              (ln2 * ln2))));
    layer.hashCount = qMax(1, static_cast<int>(
        qRound(static_cast<double>(layer.bitCount) /
               static_cast<double>(layer.capacity) * ln2)));

    int wordCount = (layer.bitCount + 63) / 64;
    layer.bits.resize(wordCount, 0);
    layer.inserted = 0;

    m_filters.append(layer);
    m_stats.totalLayers = m_filters.size();
    m_stats.estimatedFalsePositiveRate = estimateFalsePositiveRate();

    emit layerAdded(m_filters.size() - 1);
    emit falsePositiveRateChanged(m_stats.estimatedFalsePositiveRate);
}

int ScalableBloomFilter3::hashAt(const QByteArray& item, int hashIdx,
                                  int bitCount) const
{
    /* 双哈希法: h(i) = h1 + i*h2 */
    quint32 h1 = qHash(item) & 0x7FFFFFFF;
    quint32 h2 = qHash(item + QByteArray(1, static_cast<char>(hashIdx)))
                 & 0x7FFFFFFF;
    return static_cast<int>((h1 + hashIdx * h2) % bitCount);
}

void ScalableBloomFilter3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.totalLayers = m_filters.size();
    m_stats.estimatedFalsePositiveRate = estimateFalsePositiveRate();
    m_timeSum = 0.0;
}
