/**
 * @file LRUCache.h
 * @brief LRU缓存模板
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <QPair>

/**
 * @brief LRU(最近最少使用)缓存
 *
 * 高性能模板化LRU缓存,支持O(1)查找/插入/删除,
 * 统计命中率并发出淘汰信号。
 */
class LRUCache : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalGets = 0;              ///< 总查询次数
        int totalPuts = 0;              ///< 总写入次数
        int totalHits = 0;              ///< 命中次数
        int totalMisses = 0;            ///< 未命中次数
        int totalEvictions = 0;         ///< 淘汰次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit LRUCache(int capacity, QObject* parent = nullptr);

    /**
     * @brief 获取缓存值
     * @param key 键
     * @param value 输出值
     * @return 是否命中
     */
    bool get(const QString& key, QString& value);

    /**
     * @brief 写入缓存
     * @param key 键
     * @param value 值
     */
    void put(const QString& key, const QString& value);

    /**
     * @brief 删除缓存项
     */
    void remove(const QString& key);

    /**
     * @brief 检查键是否存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 清空缓存
     */
    void clear();

    /**
     * @brief 获取当前缓存大小
     */
    int size() const;

    /**
     * @brief 获取缓存容量
     */
    int capacity() const;

    /**
     * @brief 设置缓存容量
     */
    void setCapacity(int capacity);

    /**
     * @brief 获取命中率
     */
    double hitRate() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 缓存淘汰信号 */
    void evicted(const QString& key);

private:
    int m_capacity;
    QList<QPair<QString, QString>> m_list;    ///< LRU链表(前=最近)
    QMap<QString, int> m_index;               ///< key -> list位置索引
    Stats m_stats;
    double m_timeSum = 0.0;

    void moveToFront(int idx);
};
