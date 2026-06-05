/**
 * @file DataCache.h
 * @brief 数据缓存引擎 — LRU/LFU/FIFO缓存策略
 *
 * 功能: 支持LRU(最近最少使用)/LFU(最不经常使用)/FIFO(先进先出)
 *       三种缓存淘汰策略，统计命中率/峰值大小/淘汰计数。
 */
#ifndef DATACACHE2_H
#define DATACACHE2_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QPair>
#include <QString>

class DataCache : public QObject {
    Q_OBJECT
public:
    enum class EvictionPolicy { LRU, LFU, FIFO };

    struct Stats {
        quint64 totalHits = 0;
        quint64 totalMisses = 0;
        quint64 totalEvictions = 0;
        int     peakSize = 0;
        double  hitRate = 0.0;
    };

    explicit DataCache(QObject* parent = nullptr);

    void setMaxSize(int size);
    void setPolicy(EvictionPolicy policy);
    void put(const QString& key, const QByteArray& value);
    QByteArray get(const QString& key);
    bool contains(const QString& key) const;
    void remove(const QString& key);
    void clear();

    int size() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cacheHit(const QString& key);
    void cacheMiss(const QString& key);
    void eviction(const QString& key, int currentSize);

private:
    void evict();
    void updateHitRate();

    int m_maxSize;
    EvictionPolicy m_policy;
    QMap<QString, QPair<QByteArray, int>> m_data;  ///< key → (value, frequency)
    QList<QString> m_order;                          ///< LRU/FIFO顺序
};

#endif // DATACACHE2_H
