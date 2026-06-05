/**
 * @file CuckooFilter2.h
 * @brief Cuckoo Filter增强版(支持删除和计数)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class CuckooFilter2
 * @brief Cuckoo Filter — 支持删除的近似成员查询数据结构
 *
 * 比布隆过滤器更节省空间，支持删除操作。
 * 使用部分键cuckoo哈希，每个桶存储多个指纹。
 */
class CuckooFilter2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;      /**< 总插入次数 */
        int totalLookups = 0;      /**< 总查询次数 */
        int totalDeletes = 0;      /**< 总删除次数 */
        int totalHits = 0;         /**< 命中次数 */
        int totalKicks = 0;        /**< 踢出次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param capacity 预期容量(默认65536)
     * @param bucketSize 每桶条目数(默认4)
     * @param fingerprintSize 指纹位数(默认8)
     * @param maxKicks 最大踢出次数(默认500)
     * @param parent 父对象
     */
    explicit CuckooFilter2(int capacity = 65536, int bucketSize = 4,
                            int fingerprintSize = 8, int maxKicks = 500,
                            QObject* parent = nullptr);

    /** @brief 插入元素 */
    bool insert(const QByteArray& item);

    /** @brief 查询元素 */
    bool contains(const QByteArray& item) const;

    /** @brief 删除元素 */
    bool remove(const QByteArray& item);

    /** @brief 插入字符串 */
    bool insertString(const QString& str);

    /** @brief 查询字符串 */
    bool containsString(const QString& str) const;

    /** @brief 元素数量 */
    int count() const;

    /** @brief 容量 */
    int capacity() const;

    /** @brief 负载因子 */
    double loadFactor() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入成功信号 */
    void inserted(const QByteArray& item, bool success);

private:
    quint32 hash(const QByteArray& data) const;
    quint32 fingerprint(const QByteArray& data) const;
    quint32 altIndex(quint32 index, quint32 fp) const;

    QVector<quint32> m_buckets;     /**< 桶数组 */
    int m_numBuckets;               /**< 桶数 */
    int m_bucketSize;               /**< 每桶大小 */
    int m_fpSize;                   /**< 指纹大小 */
    int m_maxKicks;                 /**< 最大踢出 */
    int m_count;                    /**< 元素数 */

    Stats m_stats;
    double m_timeSum;
};
