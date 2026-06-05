/**
 * @file TinyLfuCache.h
 * @brief TinyLFU缓存 — 窗口缓存+主缓存准入策略
 *
 * 功能: TinyLFU缓存策略，使用小型窗口缓存过滤低频访问，
 *       主缓存采用LFU/SLRU混合淘汰，统计命中率/淘汰数。
 */
#ifndef TINYLFUCACHE_H
#define TINYLFUCACHE_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QPair>

class TinyLfuCache : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalAccesses = 0;
        quint64 totalHits = 0;
        quint64 totalMisses = 0;
        quint64 totalEvictions = 0;
        quint64 totalAdmissions = 0;
        double  hitRate = 0.0;
        double  avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造
     * @param capacity 总缓存容量
     * @param windowRatio 窗口缓存占比(默认0.01)
     * @param parent 父对象
     */
    explicit TinyLfuCache(int capacity = 1000, double windowRatio = 0.01,
                           QObject* parent = nullptr);

    /** @brief 访问键 @param key 键 @return 是否命中 */
    bool access(int key);

    /** @brief 插入键值 @param key 键 @param value 值 */
    void put(int key, const QByteArray& value);

    /** @brief 获取值 @param key 键 @return 值(未命中返回空) */
    QByteArray get(int key);

    /** @brief 是否包含 @param key 键 @return 包含 */
    bool contains(int key) const;

    /** @brief 移除键 @param key 键 @return 是否成功 */
    bool remove(int key);

    /** @brief 当前元素数 */
    int size() const { return m_windowSize + m_mainSize; }

    /** @brief 容量 */
    int capacity() const { return m_capacity; }

    /** @brief 清空缓存 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cacheHit(int key);
    void cacheMiss(int key);
    void evicted(int key);
    void admitted(int key);

private:
    void updateFrequency(int key);
    int frequency(int key) const;
    void evictFromWindow();
    void evictFromMain();
    void tryAdmit(int key, const QByteArray& value);

    int m_capacity;
    int m_windowCapacity;
    int m_mainCapacity;

    /* 窗口缓存: 简单LRU */
    struct CacheEntry {
        QByteArray value;
        int freq;
    };
    QHash<int, CacheEntry> m_window;
    QList<int> m_windowLru;          ///< LRU顺序

    /* 主缓存: LFU */
    QHash<int, CacheEntry> m_main;
    QHash<int, QList<int>> m_freqList;  ///< freq→keys
    int m_minFreq;

    /* 频率计数器(CM-Sketch简化) */
    QHash<int, int> m_freqCounter;
    int m_totalAccesses;  ///< 用于衰减

    int m_windowSize;
    int m_mainSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // TINYLFUCACHE_H
