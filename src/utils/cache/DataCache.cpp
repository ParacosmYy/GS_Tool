/**
 * @file DataCache.cpp
 * @brief DataCache 实现 — LRU/LFU/FIFO/TTL 淘汰策略的通用键值缓存
 */

#include "utils/cache/DataCache.h"

#include <QMutexLocker>
#include <algorithm>

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造函数，初始化时钟和配置
 * @param maxSize 最大条目数
 * @param policy  淘汰策略
 * @param parent  父对象
 */
DataCache::DataCache(int maxSize, EvictionPolicy policy, QObject *parent)
    : QObject(parent)
    , m_policy(policy)
    , m_maxSize(qMax(1, maxSize))
    , m_defaultTtlMs(0)
    , m_orderCounter(0)
{
    setObjectName(QStringLiteral("DataCache"));
    m_clock.start();
}

/**
 * @brief 析构函数 — 清空缓存，释放资源
 */
DataCache::~DataCache()
{
    clear();
}

// ============================================================================
// 存取操作
// ============================================================================

/**
 * @brief 写入缓存条目
 *
 * 如果键已存在则更新值和 TTL；如果容量已满则按当前策略淘汰。
 * @param key    键
 * @param value  值
 * @param ttlMs  生存时间（0 = 默认 TTL，-1 = 永不过期）
 */
void DataCache::put(const QString &key, const QVariant &value, qint64 ttlMs)
{
    QMutexLocker locker(&m_mutex);

    // 先清理已过期条目，为新条目腾出空间
    evictExpiredUnlocked();

    const qint64 currentTime = nowMs();

    // 若键已存在，直接更新
    if (m_entries.contains(key)) {
        CacheEntry &entry = m_entries[key];
        entry.value = value;
        entry.lastAccessMs = currentTime;
        entry.frequency += 1;
        // 更新 TTL
        if (ttlMs == -1) {
            entry.expireTimeMs = -1;
        } else {
            const qint64 effectiveTtl = (ttlMs > 0) ? ttlMs : m_defaultTtlMs;
            entry.expireTimeMs = (effectiveTtl > 0) ? currentTime + effectiveTtl : -1;
        }
        return;
    }

    // 容量已满，先发射 cacheFull 信号再淘汰
    if (m_entries.size() >= m_maxSize) {
        // 暂时释放锁以发射信号，避免死锁
        locker.unlock();
        emit cacheFull();
        locker.relock();

        // 再次检查容量（信号处理期间可能有人调用了 clear）
        if (m_entries.size() >= m_maxSize) {
            evictOneUnlocked();
        }
    }

    // 构造新条目
    CacheEntry entry;
    entry.value = value;
    entry.insertTimeMs = currentTime;
    entry.lastAccessMs = currentTime;
    entry.frequency = 1;
    entry.orderIndex = m_orderCounter++;

    if (ttlMs == -1) {
        entry.expireTimeMs = -1;
    } else {
        const qint64 effectiveTtl = (ttlMs > 0) ? ttlMs : m_defaultTtlMs;
        entry.expireTimeMs = (effectiveTtl > 0) ? currentTime + effectiveTtl : -1;
    }

    m_entries.insert(key, entry);
    m_totalInserts++;
}

/**
 * @brief 读取缓存条目
 *
 * 命中时更新 LRU/LFU 访问元数据；过期或不存在时返回无效 QVariant。
 * 统计命中/未命中次数和累计访问耗时。
 * @param key 键
 * @return 缓存值或无效 QVariant
 */
QVariant DataCache::get(const QString &key)
{
    QMutexLocker locker(&m_mutex);

    const qint64 startTime = m_clock.nsecsElapsed();

    // 不存在
    if (!m_entries.contains(key)) {
        m_totalMisses++;
        m_totalAccessTimeNs += (m_clock.nsecsElapsed() - startTime);
        return QVariant();
    }

    CacheEntry &entry = m_entries[key];
    const qint64 currentTime = nowMs();

    // 已过期 → 移除并计为 miss
    if (entry.expireTimeMs > 0 && currentTime >= entry.expireTimeMs) {
        m_entries.remove(key);
        m_totalExpirations++;
        m_totalMisses++;
        m_totalAccessTimeNs += (m_clock.nsecsElapsed() - startTime);

        // 信号在锁外发射
        locker.unlock();
        emit entryExpired(key);
        return QVariant();
    }

    // 命中：更新访问元数据
    entry.lastAccessMs = currentTime;
    entry.frequency++;

    m_totalHits++;
    m_totalAccessTimeNs += (m_clock.nsecsElapsed() - startTime);
    return entry.value;
}

/**
 * @brief 判断键是否存在且未过期
 * @param key 键
 * @return true = 存在且有效
 */
bool DataCache::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);

    if (!m_entries.contains(key))
        return false;

    const CacheEntry &entry = m_entries[key];
    if (entry.expireTimeMs > 0 && nowMs() >= entry.expireTimeMs) {
        // 注意：此处为 const 方法，不执行移除，只报告不存在
        return false;
    }
    return true;
}

/**
 * @brief 移除指定键
 * @param key 键
 */
void DataCache::remove(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    m_entries.remove(key);
}

// ============================================================================
// 容量管理
// ============================================================================

/** @brief 清空所有缓存条目 */
void DataCache::clear()
{
    QMutexLocker locker(&m_mutex);
    m_entries.clear();
    m_orderCounter = 0;
}

/** @brief 当前条目数 */
int DataCache::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_entries.size();
}

/** @brief 最大条目容量 */
int DataCache::maxSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxSize;
}

/**
 * @brief 设置最大容量
 *
 * 若新容量小于当前条目数，立即按策略淘汰多余条目。
 * @param size 新容量（必须 > 0）
 */
void DataCache::setMaxSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = qMax(1, size);
    while (m_entries.size() > m_maxSize) {
        evictOneUnlocked();
    }
}

/**
 * @brief 设置默认 TTL
 * @param ms 默认生存时间（毫秒），0 表示永不过期
 */
void DataCache::setDefaultTtl(qint64 ms)
{
    QMutexLocker locker(&m_mutex);
    m_defaultTtlMs = qMax(static_cast<qint64>(0), ms);
}

/**
 * @brief 设置淘汰策略
 *
 * 切换策略不影响已存在的条目元数据；下次淘汰决策使用新策略。
 * @param policy 目标策略
 */
void DataCache::setEvictionPolicy(EvictionPolicy policy)
{
    QMutexLocker locker(&m_mutex);
    m_policy = policy;
}

// ============================================================================
// 统计
// ============================================================================

/**
 * @brief 获取当前统计快照
 * @return CacheStats 结构体
 */
DataCache::CacheStats DataCache::stats() const
{
    QMutexLocker locker(&m_mutex);

    CacheStats s;
    s.totalHits         = m_totalHits;
    s.totalMisses       = m_totalMisses;
    s.totalEvictions    = m_totalEvictions;
    s.totalExpirations  = m_totalExpirations;
    s.totalInserts      = m_totalInserts;

    const qint64 totalAccess = m_totalHits + m_totalMisses;
    s.hitRate = (totalAccess > 0)
                    ? static_cast<double>(m_totalHits) / static_cast<double>(totalAccess)
                    : 0.0;

    s.avgAccessTimeNs = (totalAccess > 0)
                            ? static_cast<double>(m_totalAccessTimeNs) / static_cast<double>(totalAccess)
                            : 0.0;

    for (int i = 0; i < 4; ++i) {
        s.evictionsByPolicy[i] = m_evictionsByPolicy[i];
    }

    return s;
}

/** @brief 重置所有统计计数器（不影响缓存内容） */
void DataCache::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_totalHits         = 0;
    m_totalMisses       = 0;
    m_totalEvictions    = 0;
    m_totalExpirations  = 0;
    m_totalInserts      = 0;
    m_totalAccessTimeNs = 0;
    for (int i = 0; i < 4; ++i) {
        m_evictionsByPolicy[i] = 0;
    }
}

// ============================================================================
// 私有方法
// ============================================================================

/**
 * @brief 执行一次过期清理（调用者必须已持有 m_mutex）
 *
 * 遍历所有条目，移除 expireTimeMs 已到达的条目。
 * 每次 get/put 时都会调用，保持缓存整洁。
 */
void DataCache::evictExpiredUnlocked()
{
    const qint64 currentTime = nowMs();
    QStringList expiredKeys;

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if (it->expireTimeMs > 0 && currentTime >= it->expireTimeMs) {
            expiredKeys.append(it.key());
        }
    }

    for (const QString &key : expiredKeys) {
        m_entries.remove(key);
        m_totalExpirations++;
    }

    // 信号在锁外发射以避免死锁
    if (!expiredKeys.isEmpty()) {
        m_mutex.unlock();
        for (const QString &key : expiredKeys) {
            emit entryExpired(key);
        }
        m_mutex.lock();
    }
}

/**
 * @brief 按当前策略淘汰一个条目（调用者必须已持有 m_mutex）
 *
 * LRU:  淘汰 lastAccessMs 最小的条目
 * LFU:  淘汰 frequency 最小的条目（同频时淘汰最早插入的）
 * FIFO: 淘汰 orderIndex 最小的条目
 * TTL:  不执行淘汰（由过期机制处理）
 */
void DataCache::evictOneUnlocked()
{
    if (m_entries.isEmpty())
        return;

    // TTL 策略不做主动淘汰
    if (m_policy == EvictionPolicy::TTL)
        return;

    QString victimKey;
    switch (m_policy) {
    case EvictionPolicy::LRU: {
        // 淘汰最近访问时间最早的
        qint64 oldestAccess = std::numeric_limits<qint64>::max();
        for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it) {
            if (it->lastAccessMs < oldestAccess) {
                oldestAccess = it->lastAccessMs;
                victimKey = it.key();
            }
        }
        break;
    }
    case EvictionPolicy::LFU: {
        // 淘汰访问频次最低的；同频时取最早插入的
        int lowestFreq = std::numeric_limits<int>::max();
        int earliestOrder = std::numeric_limits<int>::max();
        for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it) {
            if (it->frequency < lowestFreq
                || (it->frequency == lowestFreq && it->orderIndex < earliestOrder)) {
                lowestFreq = it->frequency;
                earliestOrder = it->orderIndex;
                victimKey = it.key();
            }
        }
        break;
    }
    case EvictionPolicy::FIFO: {
        // 淘汰插入序号最小的
        int minOrder = std::numeric_limits<int>::max();
        for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it) {
            if (it->orderIndex < minOrder) {
                minOrder = it->orderIndex;
                victimKey = it.key();
            }
        }
        break;
    }
    case EvictionPolicy::TTL:
        break;
    }

    if (!victimKey.isEmpty()) {
        m_entries.remove(victimKey);
        m_totalEvictions++;
        m_evictionsByPolicy[static_cast<int>(m_policy)]++;

        // 信号在锁外发射
        m_mutex.unlock();
        emit entryEvicted(victimKey);
        m_mutex.lock();
    }
}

/**
 * @brief 当前单调时钟（毫秒）
 * @return 自缓存创建以来经过的毫秒数
 */
qint64 DataCache::nowMs() const
{
    return m_clock.elapsed();
}
