/**
 * @file TDigest.cpp
 * @brief T-Digest分位数估计器实现
 */

#include "TDigest.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

TDigest::TDigest(double delta, QObject* parent)
    : QObject(parent)
    , m_delta(delta)
    , m_minValue(std::numeric_limits<double>::max())
    , m_maxValue(std::numeric_limits<double>::lowest())
    , m_totalWeight(0.0)
    , m_compressed(true)
{
}

TDigest::~TDigest() = default;

// ═══════════════════════════════════════════════════════════
// 数据操作
// ═══════════════════════════════════════════════════════════

void TDigest::add(double value, double weight)
{
    Centroid c;
    c.mean = value;
    c.weight = weight;
    m_centroids.append(c);

    if (value < m_minValue) m_minValue = value;
    if (value > m_maxValue) m_maxValue = value;
    m_totalWeight += weight;
    m_compressed = false;

    m_stats.totalAdds++;

    // 周期性压缩
    if (m_centroids.size() > static_cast<int>(m_delta * 2)) {
        compress();
    }
}

void TDigest::addBatch(const QVector<double>& values)
{
    for (double v : values) {
        add(v);
    }
}

void TDigest::merge(const TDigest& other)
{
    for (const Centroid& c : other.m_centroids) {
        m_centroids.append(c);
    }

    if (other.m_minValue < m_minValue) m_minValue = other.m_minValue;
    if (other.m_maxValue > m_maxValue) m_maxValue = other.m_maxValue;
    m_totalWeight += other.m_totalWeight;
    m_compressed = false;

    m_stats.totalMerges++;
    compress();
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

double TDigest::quantile(double q)
{
    if (q < 0.0) q = 0.0;
    if (q > 1.0) q = 1.0;

    if (!m_compressed) compress();

    m_stats.totalQueries++;

    if (m_centroids.isEmpty()) return 0.0;
    if (m_centroids.size() == 1) return m_centroids[0].mean;

    const double result = interpolateQuantile(q);
    emit quantileQueried(q, result);
    return result;
}

double TDigest::percentile(double p)
{
    return quantile(p / 100.0);
}

double TDigest::median()
{
    return quantile(0.5);
}

double TDigest::trimmedMean(double trim)
{
    if (trim < 0.0) trim = 0.0;
    if (trim >= 0.5) return 0.0;

    const double lo = quantile(trim);
    const double hi = quantile(1.0 - trim);

    double sum = 0.0;
    double weight = 0.0;
    for (const Centroid& c : m_centroids) {
        if (c.mean >= lo && c.mean <= hi) {
            sum += c.mean * c.weight;
            weight += c.weight;
        }
    }
    return (weight > 0.0) ? (sum / weight) : 0.0;
}

// ═══════════════════════════════════════════════════════════
// 属性查询
// ═══════════════════════════════════════════════════════════

double TDigest::minimum() const { return m_minValue; }
double TDigest::maximum() const { return m_maxValue; }
double TDigest::totalWeight() const { return m_totalWeight; }
int TDigest::centroidCount() const { return m_centroids.size(); }
bool TDigest::isEmpty() const { return m_centroids.isEmpty(); }

// ═══════════════════════════════════════════════════════════
// 序列化
// ═══════════════════════════════════════════════════════════

QByteArray TDigest::serialize() const
{
    QByteArray result;
    QDataStream ds(&result, QIODevice::WriteOnly);
    ds << m_delta << m_minValue << m_maxValue << m_totalWeight;
    ds << static_cast<quint32>(m_centroids.size());
    for (const Centroid& c : m_centroids) {
        ds << c.mean << c.weight;
    }
    return result;
}

bool TDigest::deserialize(const QByteArray& data)
{
    QDataStream ds(data);
    ds >> m_delta >> m_minValue >> m_maxValue >> m_totalWeight;
    quint32 count = 0;
    ds >> count;
    m_centroids.resize(static_cast<int>(count));
    for (quint32 i = 0; i < count; ++i) {
        ds >> m_centroids[static_cast<int>(i)].mean;
        ds >> m_centroids[static_cast<int>(i)].weight;
    }
    m_compressed = true;
    return ds.status() == QDataStream::Ok;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

TDigest::Stats TDigest::stats() const
{
    Stats s = m_stats;
    s.minValue = m_minValue;
    s.maxValue = m_maxValue;
    s.totalWeight = m_totalWeight;
    return s;
}

void TDigest::resetStatistics() { m_stats = Stats{}; }

void TDigest::reset()
{
    m_centroids.clear();
    m_minValue = std::numeric_limits<double>::max();
    m_maxValue = std::numeric_limits<double>::lowest();
    m_totalWeight = 0.0;
    m_compressed = true;
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void TDigest::compress()
{
    if (m_compressed || m_centroids.size() <= 1) {
        m_compressed = true;
        return;
    }

    // 按均值排序
    std::sort(m_centroids.begin(), m_centroids.end(),
              [](const Centroid& a, const Centroid& b) {
                  return a.mean < b.mean;
              });

    QVector<Centroid> merged;
    merged.reserve(static_cast<int>(m_delta));

    Centroid current = m_centroids[0];
    double weightSoFar = current.weight;

    for (int i = 1; i < m_centroids.size(); ++i) {
        const Centroid& next = m_centroids[i];
        const double q = (weightSoFar + next.weight * 0.5) / m_totalWeight;
        const double kLimit = 4.0 * m_totalWeight * q * (1.0 - q) / m_delta;

        if (current.weight + next.weight <= kLimit) {
            // 合并
            const double newWeight = current.weight + next.weight;
            current.mean = (current.mean * current.weight +
                           next.mean * next.weight) / newWeight;
            current.weight = newWeight;
        } else {
            merged.append(current);
            current = next;
        }
        weightSoFar += next.weight;
    }
    merged.append(current);

    m_centroids = merged;
    m_compressed = true;
}

double TDigest::interpolateQuantile(double q) const
{
    const double targetWeight = q * m_totalWeight;
    double cumulativeWeight = 0.0;

    for (int i = 0; i < m_centroids.size(); ++i) {
        const Centroid& c = m_centroids[i];
        const double halfWeight = c.weight * 0.5;

        if (cumulativeWeight + halfWeight >= targetWeight) {
            // 目标在当前质心的左半部分
            if (i == 0) return m_minValue;
            const double prevEnd = cumulativeWeight;
            const double ratio = (targetWeight - prevEnd) / c.weight;
            return m_centroids[i - 1].mean +
                   ratio * (c.mean - m_centroids[i - 1].mean);
        }

        cumulativeWeight += halfWeight;

        if (cumulativeWeight + halfWeight >= targetWeight) {
            // 目标在当前质心的右半部分
            if (i == m_centroids.size() - 1) return m_maxValue;
            const double ratio = (targetWeight - cumulativeWeight) / c.weight;
            return c.mean + ratio * (m_centroids[i + 1].mean - c.mean);
        }

        cumulativeWeight += halfWeight;
    }

    return m_maxValue;
}
