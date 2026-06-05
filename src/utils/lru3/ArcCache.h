/**
 * @file ArcCache.h
 * @brief 自适应替换缓存(ARC) — 平衡频率与最近性的智能缓存
 *
 * 实现ARC(Adaptive Replacement Cache)算法, 自动在最近使用(LRU)
 * 和频繁使用(LFU)策略间动态调整, 适用于嵌入式调试场景中的
 * 数据缓存和协议解析结果复用。
 */
#ifndef ARC_CACHE_H
#define ARC_CACHE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QHash>
#include <list>
#include <utility>

/**
 * @class ArcCache
 * @brief 自适应替换缓存(ARC)
 *
 * 典型用法:
 * @code
 *   ArcCache cache;
 *   cache.setCapacity(256);
 *   cache.put("key", value);
 *   QVariant val = cache.get("key");
 * @endcode
 */
class ArcCache : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalGets = 0;              ///< 查询操作总次数
        quint64 totalPuts = 0;              ///< 存入操作总次数
        quint64 totalHits = 0;              ///< 缓存命中次数
        quint64 totalMisses = 0;            ///< 缓存未命中次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit ArcCache(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ArcCache() override;

    // ── 配置 ──

    /**
     * @brief 设置缓存容量
     * @param capacity 最大缓存条目数
     */
    void setCapacity(int capacity);

    /** @brief 获取当前缓存容量 */
    int capacity() const;

    // ── 缓存操作 ──

    /**
     * @brief 获取缓存值
     * @param key 缓存键
     * @return 缓存值; 未命中返回无效QVariant
     */
    QVariant get(const QString& key);

    /**
     * @brief 存入缓存值
     * @param key 缓存键
     * @param value 缓存值
     */
    void put(const QString& key, const QVariant& value);

    /**
     * @brief 检查键是否存在
     * @param key 缓存键
     * @return true键存在
     */
    bool contains(const QString& key) const;

    /** @brief 清空缓存 */
    void clear();

    /** @brief 当前缓存条目数 */
    int size() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 缓存命中信号 @param key 命中的键 */
    void cacheHit(const QString& key);
    /** @brief 缓存未命中信号 @param key 未命中的键 */
    void cacheMiss(const QString& key);
    /** @brief 缓存驱逐信号 @param key 被驱逐的键 */
    void evicted(const QString& key);

private:
    /** @brief LRU列表类型 */
    using EntryList = std::list<std::pair<QString, QVariant>>;

    /** @brief 迭代器类型 */
    using EntryIter = EntryList::iterator;

    /**
     * @brief 将条目移到列表头部
     * @param list 目标列表
     * @param it 条目迭代器
     */
    void moveToFront(EntryList& list, EntryIter it);

    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief T1列表: 最近使用一次的条目 */
    EntryList m_t1;

    /** @brief T2列表: 最近使用多次的条目 */
    EntryList m_t2;

    /** @brief B1列表: 被驱逐的T1幽灵条目 */
    EntryList m_b1;

    /** @brief B2列表: 被驱逐的T2幽灵条目 */
    EntryList m_b2;

    /** @brief T1键到迭代器的映射 */
    QHash<QString, EntryIter> m_t1Map;

    /**
     * @brief ARC替换策略: 当缓存满时从T1或T2淘汰
     * @param key 引起替换的键
     */
    void arcReplace(const QString& key);

    /** @brief T2键到迭代器的映射 */
    QHash<QString, EntryIter> m_t2Map;

    /** @brief B1键集合 */
    QHash<QString, bool> m_b1Set;

    /** @brief B2键集合 */
    QHash<QString, bool> m_b2Set;

    /** @brief 目标容量 */
    int m_capacity = 256;

    /** @brief 自适应参数p */
    int m_p = 0;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

#endif // ARC_CACHE_H
