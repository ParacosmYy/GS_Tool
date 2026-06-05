/**
 * @file DataCache.h
 * @brief 数据缓存 — 支持 LRU/LFU/FIFO/TTL 淘汰策略的通用键值缓存
 *
 * 为串口数据处理结果提供带过期和淘汰机制的缓存，避免重复计算。
 * 线程安全（QMutex 保护），可配置最大容量、默认 TTL 和淘汰策略。
 */
#ifndef DATA_CACHE_H
#define DATA_CACHE_H

#include <QObject>
#include <QMutex>
#include <QVariant>
#include <QElapsedTimer>
#include <QMap>
#include <QList>
#include <QPair>

/** @brief 淘汰策略枚举: LRU/最近最少, LFU/最不常用, FIFO/先进先出, TTL/仅过期 */
enum class EvictionPolicy { LRU = 0, LFU = 1, FIFO = 2, TTL = 3 };

/**
 * @class DataCache
 * @brief 通用数据缓存，支持四种淘汰策略与 TTL 过期
 *
 * 典型用途：缓存协议解析结果、校验和计算结果、数据转换中间值，
 * 在重复输入相同时直接返回缓存值，减少 CPU 开销。
 * 线程约束：所有公开方法内部使用 QMutex，可跨线程安全调用。
 * 信号在锁释放后发射，避免死锁。
 */
class DataCache : public QObject
{
    Q_OBJECT

public:
    /** @brief 缓存统计信息：命中率、淘汰计数、平均访问耗时等运行指标 */
    struct CacheStats {
        qint64 totalHits         = 0;   ///< 命中次数
        qint64 totalMisses       = 0;   ///< 未命中次数
        qint64 totalEvictions    = 0;   ///< 淘汰总次数
        qint64 totalExpirations  = 0;   ///< TTL 过期总次数
        qint64 totalInserts      = 0;   ///< 插入总次数
        double hitRate           = 0.0; ///< 命中率 [0.0, 1.0]
        double avgAccessTimeNs   = 0.0; ///< 平均单次访问耗时（纳秒）
        qint64 evictionsByPolicy[4] = {}; ///< 按策略索引(EvictionPolicy)的淘汰计数
    };

    /**
     * @brief 构造函数
     * @param maxSize 最大缓存条目数（默认 256）
     * @param policy  淘汰策略（默认 LRU）
     * @param parent  父对象
     */
    explicit DataCache(int maxSize = 256,
                       EvictionPolicy policy = EvictionPolicy::LRU,
                       QObject *parent = nullptr);
    ~DataCache() override;

    DataCache(const DataCache&) = delete;
    DataCache& operator=(const DataCache&) = delete;

    /** @brief 写入缓存条目; ttlMs=0使用默认TTL, -1永不过期 */
    void put(const QString &key, const QVariant &value, qint64 ttlMs = 0);

    /** @brief 读取缓存条目; 不存在或已过期返回无效 QVariant */
    QVariant get(const QString &key);

    /** @brief 判断键是否存在且未过期 */
    bool contains(const QString &key) const;

    /** @brief 移除指定键 */
    void remove(const QString &key);

    /** @brief 清空所有缓存条目 */
    void clear();
    /** @brief 当前条目数 */
    int size() const;
    /** @brief 最大条目容量 */
    int maxSize() const;

    /** @brief 设置最大容量; 若新容量不足，立即按策略淘汰多余条目 */
    void setMaxSize(int size);
    /** @brief 设置默认TTL（毫秒），0 表示永不过期 */
    void setDefaultTtl(qint64 ms);
    /** @brief 设置淘汰策略; 切换策略不会立即重排现有条目 */
    void setEvictionPolicy(EvictionPolicy policy);

    /** @brief 获取当前统计快照 */
    CacheStats stats() const;
    /** @brief 重置所有统计计数器（不影响缓存内容） */
    void resetStatistics();

signals:
    /** @brief 条目因淘汰被移除时发射 */
    void entryEvicted(const QString &key);
    /** @brief 条目因 TTL 过期被移除时发射 */
    void entryExpired(const QString &key);
    /** @brief 缓存已满、即将淘汰条目前发射 */
    void cacheFull();

private:
    /** @brief 单条缓存条目 */
    struct CacheEntry {
        QVariant value;           ///< 缓存值
        qint64   insertTimeMs;    ///< 插入时刻（自启动的毫秒数）
        qint64   expireTimeMs;    ///< 过期时刻，-1 永不过期
        qint64   lastAccessMs;    ///< 最近访问时刻（LRU 使用）
        int      frequency;       ///< 访问频次（LFU 使用）
        int      orderIndex;      ///< 插入序号（FIFO 使用）
    };

    void evictExpiredUnlocked();  ///< 过期清理（锁已持有）
    void evictOneUnlocked();      ///< 按策略淘汰一个（锁已持有）
    qint64 nowMs() const;         ///< 当前单调时钟（毫秒）

    mutable QMutex            m_mutex;
    QMap<QString, CacheEntry> m_entries;
    EvictionPolicy            m_policy        = EvictionPolicy::LRU;
    int                       m_maxSize       = 256;
    qint64                    m_defaultTtlMs  = 0;
    int                       m_orderCounter  = 0;
    QElapsedTimer             m_clock;

    mutable qint64 m_totalHits         = 0;
    mutable qint64 m_totalMisses       = 0;
    mutable qint64 m_totalEvictions    = 0;
    mutable qint64 m_totalExpirations  = 0;
    mutable qint64 m_totalInserts      = 0;
    mutable qint64 m_totalAccessTimeNs = 0;
    mutable qint64 m_evictionsByPolicy[4] = {};
};

#endif // DATA_CACHE_H
