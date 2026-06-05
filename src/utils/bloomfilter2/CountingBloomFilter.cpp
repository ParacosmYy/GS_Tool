/**
 * @file CountingBloomFilter.cpp
 * @brief 计数型布隆过滤器实现 — 饱和计数器/多哈希
 */

#include "utils/bloomfilter2/CountingBloomFilter.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param expectedElements 预期元素 @param falsePositiveRate 误判率 @param parent 父对象 */
CountingBloomFilter2::CountingBloomFilter2(int expectedElements,
                                         double falsePositiveRate,
                                         QObject* parent)
    : QObject(parent)
    , m_elementCount(0)
    , m_timeSum(0.0)
{
    computeOptimalParams(expectedElements, falsePositiveRate);
    m_counters.fill(0, m_bitSize);
}

/** @brief 计算最优参数 @param n 元素数 @param fpRate 误判率 */
void CountingBloomFilter2::computeOptimalParams(int n, double fpRate)
{
    if (n <= 0) n = 10000;
    if (fpRate <= 0 || fpRate >= 1) fpRate = 0.01;

    /* m = -n * ln(p) / (ln(2)^2) */
    double ln2 = qLn(2.0);
    m_bitSize = static_cast<int>(qCeil(-static_cast<double>(n) * qLn(fpRate)
                                       / (ln2 * ln2)));
    if (m_bitSize < 64) m_bitSize = 64;

    /* k = (m/n) * ln(2) */
    m_numHashes = static_cast<int>(qCeil(static_cast<double>(m_bitSize)
                                         / n * ln2));
    if (m_numHashes < 1) m_numHashes = 1;
    if (m_numHashes > 20) m_numHashes = 20;
}

/** @brief 添加元素 @param value 元素值 */
void CountingBloomFilter2::add(int value)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> positions = hashPositions(value);
    for (int pos : positions) {
        if (m_counters[pos] < MAX_COUNTER) {
            ++m_counters[pos];
        }
    }
    ++m_elementCount;

    m_stats.totalAdds++;
    m_stats.estimatedElements = m_elementCount;
    m_stats.estimatedFpRate = estimatedFalsePositiveRate();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalLookups;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementAdded(value);
}

/** @brief 移除元素 @param value 元素值 @return 是否成功 */
bool CountingBloomFilter2::remove(int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 先检查是否存在 */
    QVector<int> positions = hashPositions(value);
    for (int pos : positions) {
        if (m_counters[pos] == 0) {
            /* 元素不存在 */
            m_stats.totalRemoves++;
            m_timeSum += timer.elapsed();
            emit elementRemoved(value, false);
            return false;
        }
    }

    /* 递减计数器 */
    for (int pos : positions) {
        if (m_counters[pos] > 0) {
            --m_counters[pos];
        }
    }
    --m_elementCount;
    if (m_elementCount < 0) m_elementCount = 0;

    m_stats.totalRemoves++;
    m_stats.estimatedElements = m_elementCount;
    m_stats.estimatedFpRate = estimatedFalsePositiveRate();
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalLookups;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementRemoved(value, true);
    return true;
}

/** @brief 查询是否可能包含 @param value 元素值 @return 是否可能包含 */
bool CountingBloomFilter2::contains(int value)
{
    QVector<int> positions = hashPositions(value);
    for (int pos : positions) {
        if (m_counters[pos] == 0) return false;
    }
    m_stats.totalLookups++;
    return true;
}

/** @brief 批量添加 @param values 元素列表 */
void CountingBloomFilter2::addBatch(const QVector<int>& values)
{
    for (int v : values) {
        add(v);
    }
}

/** @brief 清空过滤器 */
void CountingBloomFilter2::clear()
{
    m_counters.fill(0, m_bitSize);
    m_elementCount = 0;
    m_stats.estimatedElements = 0;
    m_stats.estimatedFpRate = 0.0;
}

/** @brief 估计元素数 @return 元素数 */
int CountingBloomFilter2::estimatedElementCount() const
{
    return m_elementCount;
}

/** @brief 估计误判率 @return 误判率 */
double CountingBloomFilter2::estimatedFalsePositiveRate() const
{
    if (m_elementCount <= 0) return 0.0;
    /* p = (1 - e^(-kn/m))^k */
    double exponent = -static_cast<double>(m_numHashes) * m_elementCount
                      / m_bitSize;
    double p = 1.0 - qExp(exponent);
    return qPow(p, m_numHashes);
}

/** @brief 过滤器容量 @return 位数组大小 */
int CountingBloomFilter2::capacity() const { return m_bitSize; }

/** @brief 哈希函数数 @return k值 */
int CountingBloomFilter2::hashCount() const { return m_numHashes; }

/** @brief 计算k个哈希位置 @param value 元素 @return 位置列表 */
QVector<int> CountingBloomFilter2::hashPositions(int value) const
{
    QVector<int> positions(m_numHashes);
    quint32 h1 = murmurHash(value, 0);
    quint32 h2 = murmurHash(value, h1);

    for (int i = 0; i < m_numHashes; ++i) {
        quint32 hi = h1 + static_cast<quint32>(i) * h2;
        positions[i] = static_cast<int>(hi % static_cast<quint32>(m_bitSize));
    }
    return positions;
}

/** @brief MurmurHash3变体 @param value 输入 @param seed 种子 @return 哈希值 */
quint32 CountingBloomFilter2::murmurHash(int value, int seed) const
{
    quint32 h = static_cast<quint32>(seed);
    quint32 k = static_cast<quint32>(value);

    /* 单轮Murmur3混合 */
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    h ^= k;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;

    /* 终结 */
    h ^= 4;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

/** @brief 重置统计 */
void CountingBloomFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
