/**
 * @file ArcCache2.h
 * @brief 自适应替换缓存(ARC) — 平衡最近性与频率的智能缓存策略
 *
 * 实现ARC(Adaptive Replacement Cache)算法，维护四个链表:
 * T1(最近使用项)、T2(频繁使用项)、B1(T1的淘汰历史)、B2(T2的淘汰历史)。
 * 通过B1/B2命中率自适应调整T1/T2的目标大小p，自动适应工作负载特征。
 */
#pragma once

#include <QObject>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QElapsedTimer>

/**
 * @class ArcCache2
 * @brief 自适应替换缓存(ARC) — 自动平衡LRU/LFU的智能缓存
 *
 * 典型用法:
 * @code
 *   ArcCache2 cache(256);
 *   cache.put("key", result);
 *   QVariant val = cache.get("key");
 * @endcode
 */
class ArcCache2 : public QObject {
    Q_OBJECT

public:
    /** @brief 缓存运行统计结构 */
    struct Stats {
        quint64 totalGets             = 0;    ///< get操作总次数
        quint64 totalHits             = 0;    ///< 缓存命中次数
        quint64 totalPuts             = 0;    ///< put操作总次数
        double  hitRate() const {               ///< 命中率[0.0, 1.0]
            return (totalGets > 0)
                ? static_cast<double>(totalHits) / static_cast<double>(totalGets)
                : 0.0;
        }
        double  avgProcessingTimeMs   = 0.0;  ///< 平均操作耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param capacity 缓存容量(默认256)
     * @param parent   父对象
     */
    explicit ArcCache2(int capacity = 256, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ArcCache2() override;

    // ── 核心操作 ──

    /**
     * @brief 读取缓存值
     *
     * 查找T1+T2中是否存在该键, 存在则提升到T2并返回值。
     * @param key 缓存键
     * @return 缓存值; 不存在返回无效QVariant
     */
    QVariant get(const QString& key);

    /**
     * @brief 写入缓存条目
     *
     * 若键已存在于T1/T2则更新值并提升到T2。
     * 否则执行ARC替换策略: 根据B1/B2历史自适应调整p值后插入T1。
     * @param key   缓存键
     * @param value 缓存值
     */
    void put(const QString& key, const QVariant& value);

    /**
     * @brief 判断键是否存在于缓存中(T1或T2)
     * @param key 缓存键
     * @return true = 存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除指定键(从T1/T2/B1/B2中查找并移除)
     * @param key 缓存键
     * @return true = 成功移除
     */
    bool remove(const QString& key);

    /** @brief 清空所有缓存(T1/T2/B1/B2) */
    void clear();

    /** @brief 当前缓存中的有效条目数(T1+T2) @return 条目数 */
    int size() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零(不改变缓存内容) */
    void resetStatistics();

signals:
    /** @brief 条目被替换淘汰时发射 @param key 被淘汰的键 */
    void entryEvicted(const QString& key);
    /** @brief 缓存已满、即将执行替换策略 */
    void cacheFull();

private:
    /** @brief 替换(Replacement)策略: 当T1+T2已满时淘汰一个条目 */
    void replace(bool inB2);

    /** @brief 更新put操作的统计 */
    void updateStats();

    QHash<QString, QVariant> m_t1Data;     ///< T1: 最近使用项的数据
    QHash<QString, QVariant> m_t2Data;     ///< T2: 频繁使用项的数据
    QList<QString>           m_t1;         ///< T1: 最近使用项的LRU队列(尾=最近)
    QList<QString>           m_t2;         ///< T2: 频繁使用项的LRU队列(尾=最近)
    QList<QString>           m_b1;         ///< B1: T1淘汰历史(ghost entries)
    QList<QString>           m_b2;         ///< B2: T2淘汰历史(ghost entries)

    int  m_capacity;                        ///< 总容量c
    int  m_p;                               ///< T1目标大小(自适应参数)
    Stats  m_stats;                         ///< 统计数据
    QElapsedTimer m_timer;                  ///< 耗时计时器
};
