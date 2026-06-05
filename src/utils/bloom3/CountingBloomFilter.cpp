/**
 * @file FrequencyBloomFilter.cpp
 * @brief 计数型布隆过滤器实现 — MurmurHash3多哈希+计数器数组
 */

#include "utils/bloom3/CountingBloomFilter.h"

#include <QtGlobal>
#include <QtMath>

/** @brief 构造函数 @param expectedItems 预期元素数 @param fpRate 误判率 @param parent 父对象 */
FrequencyBloomFilter::FrequencyBloomFilter(quint64 expectedItems,
                                         double fpRate,
                                         QObject* parent)
    : QObject(parent)
    , m_expectedItems(qMax(expectedItems, quint64(100)))
    , m_fpRate(qBound(1e-6, fpRate, 0.5))
    , m_timeSum(0.0)
{
    /* 计算最优参数: m = -n*ln(p) / (ln2)^2, k = (m/n)*ln2 */
    double ln2Sq = qLn(2.0) * qLn(2.0);
    m_counterCount = static_cast<quint64>(
        qCeil(-static_cast<double>(m_expectedItems) * qLn(m_fpRate) / ln2Sq));
    m_counterCount = qBound(quint64(64), m_counterCount, quint64(1 << 24));

    m_hashCount = static_cast<int>(
        qCeil(static_cast<double>(m_counterCount)
              / static_cast<double>(m_expectedItems) * qLn(2.0)));
    m_hashCount = qBound(1, m_hashCount, 32);

    m_counters.resize(static_cast<int>(m_counterCount));
    m_counters.fill(0);
}

/** @brief 添加元素 @param item 数据项 */
void FrequencyBloomFilter::add(const QByteArray& item)
{
    m_timer.start();

    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(item, i) % static_cast<quint32>(m_counterCount);
        /* 防止计数器溢出 */
        if (m_counters[static_cast<int>(pos)] < 0xFFFF) {
            ++m_counters[static_cast<int>(pos)];
        }
    }

    ++m_stats.totalAdds;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAdds + m_stats.totalQueries);

    emit elementAdded(m_stats.totalAdds);
}

/** @brief 添加元素(QString) @param item 字符串 */
void FrequencyBloomFilter::add(const QString& item)
{
    add(item.toUtf8());
}

/** @brief 查询估计频率 @param item 数据项 @return 估计次数 */
int FrequencyBloomFilter::count(const QByteArray& item) const
{
    m_timer.start();

    int minCount = 0x7FFFFFFF;
    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(item, i) % static_cast<quint32>(m_counterCount);
        int c = m_counters[static_cast<int>(pos)];
        if (c < minCount) {
            minCount = c;
            if (minCount == 0) break; /* 短路优化 */
        }
    }

    ++m_stats.totalQueries;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAdds + m_stats.totalQueries);

    return (minCount == 0x7FFFFFFF) ? 0 : minCount;
}

/** @brief 查询估计频率(QString) @param item 字符串 @return 估计次数 */
int FrequencyBloomFilter::count(const QString& item) const
{
    return count(item.toUtf8());
}

/** @brief 移除元素 @param item 数据项 @return 是否成功 */
bool FrequencyBloomFilter::remove(const QByteArray& item)
{
    /* 先检查是否存在 */
    if (!contains(item)) {
        emit elementRemoved(false);
        return false;
    }

    m_timer.start();

    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(item, i) % static_cast<quint32>(m_counterCount);
        if (m_counters[static_cast<int>(pos)] > 0) {
            --m_counters[static_cast<int>(pos)];
        }
    }

    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAdds + m_stats.totalQueries);

    emit elementRemoved(true);
    return true;
}

/** @brief 检查元素是否存在 @param item 数据项 @return 是否可能存在 */
bool FrequencyBloomFilter::contains(const QByteArray& item) const
{
    m_timer.start();

    bool found = true;
    for (int i = 0; i < m_hashCount; ++i) {
        quint32 pos = hashN(item, i) % static_cast<quint32>(m_counterCount);
        if (m_counters[static_cast<int>(pos)] == 0) {
            found = false;
            break;
        }
    }

    ++m_stats.totalQueries;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAdds + m_stats.totalQueries);

    return found;
}

/** @brief 清空过滤器 */
void FrequencyBloomFilter::clear()
{
    m_counters.fill(0);
}

/** @brief 重置统计 */
void FrequencyBloomFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算第n个哈希值 @param data 数据 @param n 哈希序号 @return 哈希位置 */
quint32 FrequencyBloomFilter::hashN(const QByteArray& data, int n) const
{
    /* 双哈希: h1 + n * h2，模拟多个独立哈希函数 */
    quint32 h1 = murmurHash(data, 0);
    quint32 h2 = murmurHash(data, h1);
    return h1 + static_cast<quint32>(n) * h2;
}

/** @brief MurmurHash3变体 @param data 数据 @param seed 种子 @return 哈希值 */
quint32 FrequencyBloomFilter::murmurHash(const QByteArray& data,
                                         quint32 seed) const
{
    const quint8* bytes = reinterpret_cast<const quint8*>(data.constData());
    int len = data.size();
    quint32 h = seed;
    quint32 c1 = 0xcc9e2d51;
    quint32 c2 = 0x1b873593;

    /* 每次处理4字节 */
    int nBlocks = len / 4;
    for (int i = 0; i < nBlocks; ++i) {
        quint32 k = static_cast<quint32>(bytes[i * 4])
                  | (static_cast<quint32>(bytes[i * 4 + 1]) << 8)
                  | (static_cast<quint32>(bytes[i * 4 + 2]) << 16)
                  | (static_cast<quint32>(bytes[i * 4 + 3]) << 24);

        k *= c1;
        k = (k << 15) | (k >> 17); /* ROTL32 */
        k *= c2;

        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    /* 处理剩余字节 */
    const quint8* tail = bytes + nBlocks * 4;
    quint32 k1 = 0;
    switch (len & 3) {
    case 3: k1 ^= static_cast<quint32>(tail[2]) << 16; [[fallthrough]];
    case 2: k1 ^= static_cast<quint32>(tail[1]) << 8;  [[fallthrough]];
    case 1:
        k1 ^= static_cast<quint32>(tail[0]);
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        h ^= k1;
        break;
    default:
        break;
    }

    /* 最终混合 */
    h ^= static_cast<quint32>(len);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}
