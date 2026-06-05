/**
 * @file CuckooFilterV2.cpp
 * @brief 布谷鸟过滤器实现 — 有界假阳性率的集合成员查询
 */

#include "utils/hash2/CuckooFilter.h"

#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QtMath>
#include <algorithm>
#include <random>

CuckooFilterV2::CuckooFilterV2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool CuckooFilterV2::initialize(const Config& config)
{
    m_config = config;

    /* bucket数量调整为2的幂 */
    int powerOfTwo = 1;
    while (powerOfTwo < m_config.bucketCount) powerOfTwo <<= 1;
    m_config.bucketCount = powerOfTwo;

    /* 初始化bucket数组 */
    m_buckets.resize(m_config.bucketCount);
    for (auto& bucket : m_buckets) {
        bucket.resize(m_config.entriesPerBucket, 0);
    }

    m_itemCount = 0;
    return true;
}

/** @brief 初始化(默认配置) */
bool CuckooFilterV2::initialize()
{
    return initialize(Config{});
}

bool CuckooFilterV2::insert(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (m_buckets.isEmpty()) {
        if (!initialize()) return false;
    }

    /* 计算指纹和候选bucket */
    quint32 fp = fingerprint(data);
    quint32 i1 = primaryIndex(data);
    quint32 i2 = alternateIndex(i1, fp);

    /* 尝试插入到两个候选bucket */
    if (insertToBucket(i1, fp)) {
        ++m_itemCount;
        ++m_stats.totalInsertions;
        m_timeSum += timer.elapsed();
        return true;
    }

    if (insertToBucket(i2, fp)) {
        ++m_itemCount;
        ++m_stats.totalInsertions;
        m_timeSum += timer.elapsed();
        return true;
    }

    /* 两个bucket都满, 执行踢出(kickout) */
    quint32 currentIdx = i1;
    quint32 currentFp = fp;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    for (int kick = 0; kick < m_config.maxKicks; ++kick) {
        /* 随机选择一个bucket中的槽位 */
        int bucket = (kick % 2 == 0) ? currentIdx : currentIdx;
        int slot = dist(gen) % m_config.entriesPerBucket;

        /* 交换 */
        std::swap(currentFp, m_buckets[bucket][slot]);

        /* 计算被踢出指纹的备用位置 */
        currentIdx = alternateIndex(bucket, currentFp);

        /* 尝试插入到备用位置 */
        if (insertToBucket(currentIdx, currentFp)) {
            ++m_itemCount;
            ++m_stats.totalInsertions;
            m_timeSum += timer.elapsed();
            return true;
        }
    }

    /* 踢出次数用尽, 插入失败 */
    ++m_stats.totalInsertions;
    ++m_stats.totalFailedInsertions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0)
        ? m_timeSum / m_stats.totalInsertions : 0.0;

    return false;
}

bool CuckooFilterV2::contains(const QByteArray& data) const
{
    if (m_buckets.isEmpty() || m_itemCount == 0) return false;

    quint32 fp = fingerprint(data);
    quint32 i1 = primaryIndex(data);
    quint32 i2 = alternateIndex(i1, fp);

    /* 检查两个候选bucket */
    if (findInBucket(i1, fp) >= 0) return true;
    if (findInBucket(i2, fp) >= 0) return true;

    return false;
}

bool CuckooFilterV2::remove(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (m_buckets.isEmpty() || m_itemCount == 0) return false;

    quint32 fp = fingerprint(data);
    quint32 i1 = primaryIndex(data);
    quint32 i2 = alternateIndex(i1, fp);

    /* 在bucket1中查找并删除 */
    int slot = findInBucket(i1, fp);
    if (slot >= 0) {
        m_buckets[i1][slot] = 0;
        --m_itemCount;
        ++m_stats.totalDeletions;
        m_timeSum += timer.elapsed();
        return true;
    }

    /* 在bucket2中查找并删除 */
    slot = findInBucket(i2, fp);
    if (slot >= 0) {
        m_buckets[i2][slot] = 0;
        --m_itemCount;
        ++m_stats.totalDeletions;
        m_timeSum += timer.elapsed();
        return true;
    }

    m_timeSum += timer.elapsed();
    return false;
}

int CuckooFilterV2::batchInsert(const QVector<QByteArray>& items)
{
    int success = 0;
    for (const auto& item : items) {
        if (insert(item)) ++success;
    }
    return success;
}

void CuckooFilterV2::clear()
{
    for (auto& bucket : m_buckets) {
        bucket.fill(0);
    }
    m_itemCount = 0;
}

CuckooFilterV2::CapacityInfo CuckooFilterV2::capacityInfo() const
{
    CapacityInfo info;
    info.totalSlots = m_config.bucketCount * m_config.entriesPerBucket;
    info.usedSlots = m_itemCount;
    info.loadFactor = (info.totalSlots > 0)
        ? static_cast<double>(m_itemCount) / info.totalSlots : 0.0;
    info.estimatedFPR = estimateFalsePositiveRate(
        m_config.fingerprintBits, m_config.entriesPerBucket);
    info.remainingCapacity = info.totalSlots - info.usedSlots;
    return info;
}

CuckooFilterV2::Config CuckooFilterV2::config() const
{
    return m_config;
}

double CuckooFilterV2::estimateFalsePositiveRate(int fpBits, int entriesPerBucket) const
{
    /* 布谷鸟过滤器假阳性率上限: 2b / 2^f
     * 其中 b = entriesPerBucket, f = fingerprintBits */
    double fpSpace = qPow(2.0, fpBits);
    return static_cast<double>(2 * entriesPerBucket) / fpSpace;
}

CuckooFilterV2::Config CuckooFilterV2::optimalConfig(int targetCapacity,
                                                   double targetFPR) const
{
    Config config;

    /* 根据假阳性率确定指纹位数: 2b / 2^f <= targetFPR */
    /* 取 b = 4 (标准值), 则 f >= log2(2b / FPR) */
    config.entriesPerBucket = 4;
    double minBits = qLn(2.0 * config.entriesPerBucket / targetFPR) / qLn(2.0);
    config.fingerprintBits = qCeil(minBits);
    config.fingerprintBits = qMax(4, qMin(config.fingerprintBits, 32));

    /* bucket数量: 目标容量 / 负载因子 / 每bucket槽位数 */
    /* 目标负载因子约 95% (布谷鸟过滤器典型值) */
    double targetLoad = 0.95;
    int neededBuckets = qCeil(targetCapacity / (config.entriesPerBucket * targetLoad));

    /* 调整为2的幂 */
    int powerOfTwo = 1;
    while (powerOfTwo < neededBuckets) powerOfTwo <<= 1;
    config.bucketCount = powerOfTwo;
    config.maxKicks = 500;

    return config;
}

quint32 CuckooFilterV2::fingerprint(const QByteArray& data) const
{
    /* 使用SHA256的截断作为指纹 */
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    quint32 fp = 0;
    int bytesNeeded = (m_config.fingerprintBits + 7) / 8;
    bytesNeeded = qMin(bytesNeeded, hash.size());

    for (int i = 0; i < bytesNeeded; ++i) {
        fp = (fp << 8) | static_cast<unsigned char>(hash[i]);
    }

    /* 截断到指定位数, 确保非零 */
    quint32 mask = (m_config.fingerprintBits >= 32)
        ? 0xFFFFFFFF : (1u << m_config.fingerprintBits) - 1;
    fp = (fp & mask);
    return (fp == 0) ? 1 : fp; /* 0表示空槽, 指纹不能为0 */
}

quint32 CuckooFilterV2::primaryIndex(const QByteArray& data) const
{
    /* 使用SHA256的另一部分作为主索引 */
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    quint32 idx = 0;
    for (int i = 0; i < 4 && i + 4 < hash.size(); ++i) {
        idx = (idx << 8) | static_cast<unsigned char>(hash[i + 4]);
    }

    return idx % m_config.bucketCount;
}

quint32 CuckooFilterV2::alternateIndex(quint32 primary, quint32 fp) const
{
    /* 部分键布谷鸟哈希: i2 = i1 XOR hash(fp) */
    /* 使用简单的hash以避免依赖MurmurHash */
    quint32 h = fp * 0x5bd1e995;
    h ^= h >> 13;
    h *= 0x5bd1e995;
    h ^= h >> 15;

    return (primary ^ h) % m_config.bucketCount;
}

int CuckooFilterV2::findInBucket(int bucketIdx, quint32 fp) const
{
    if (bucketIdx < 0 || bucketIdx >= m_buckets.size()) return -1;

    const auto& bucket = m_buckets[bucketIdx];
    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket[i] == fp) return i;
    }
    return -1;
}

bool CuckooFilterV2::insertToBucket(int bucketIdx, quint32 fp)
{
    if (bucketIdx < 0 || bucketIdx >= m_buckets.size()) return false;

    auto& bucket = m_buckets[bucketIdx];
    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket[i] == 0) {
            bucket[i] = fp;
            return true;
        }
    }
    return false;
}

CuckooFilterV2::Stats CuckooFilterV2::stats() const
{
    return m_stats;
}

void CuckooFilterV2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
