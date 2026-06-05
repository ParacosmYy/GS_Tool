/**
 * @file LfuCache.h
 * @brief LFU(Least Frequently Used)缓存
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>

/**
 * @class LfuCache
 * @brief LFU缓存 — 淘汰最不常使用的条目
 *
 * 使用频率桶+双向链表实现O(1)的get/put操作。
 * 同频率内按LRU淘汰(最久未使用的先淘汰)。
 */
class LfuCache : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalGets = 0;          /**< 总获取次数 */
        int totalPuts = 0;          /**< 总写入次数 */
        int totalHits = 0;          /**< 命中次数 */
        int totalEvictions = 0;     /**< 驱逐次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param capacity 容量
     * @param parent 父对象
     */
    explicit LfuCache(int capacity = 256, QObject* parent = nullptr);

    /** @brief 获取值 */
    QVariant get(const QString& key);

    /** @brief 写入值 */
    void put(const QString& key, const QVariant& value);

    /** @brief 是否包含 */
    bool contains(const QString& key) const;

    /** @brief 删除条目 */
    bool remove(const QString& key);

    /** @brief 清空 */
    void clear();

    /** @brief 当前大小 */
    int size() const;

    /** @brief 命中率 */
    double hitRate() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 条目驱逐信号 */
    void evicted(const QString& key);

private:
    struct Entry {
        QString key;
        QVariant value;
        int freq;
    };

    int m_capacity;
    QMap<QString, QList<Entry>::iterator> m_keyMap;
    QMap<int, QList<Entry>> m_freqMap;
    int m_minFreq;
    int m_count;

    Stats m_stats;
    double m_timeSum;
};
