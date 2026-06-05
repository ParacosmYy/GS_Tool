/**
 * @file CuckooHashTable.h
 * @brief 布谷鸟哈希表 — O(1)最坏情况查找的哈希表
 *
 * 功能: 使用两个哈希函数和两个表实现布谷鸟哈希，保证最坏情况下
 *       O(1)查找时间。支持插入、查找、删除操作，自动处理踢出和重哈希。
 *
 * 协作: HashMap(标准哈希) / BloomFilter(概率查询)
 */
#ifndef CUCKOOHASHTABLE_H
#define CUCKOOHASHTABLE_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief 布谷鸟哈希表
 *
 * 每个key映射到两个候选位置，查找只需检查两个位置。
 * 插入冲突时踢出已有元素使其重新定位，超过最大踢出次数则扩容重哈希。
 */
class CuckooHashTable : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalLookups = 0;          ///< 累计查找次数
        quint64 totalKicks = 0;            ///< 累计踢出次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit CuckooHashTable(int initialSize = 64,
                              QObject* parent = nullptr);

    /**
     * @brief 插入键值对
     * @param key 整数键
     * @param value 字节数组值
     */
    void insert(int key, const QByteArray& value);

    /**
     * @brief 查找键对应的值
     * @param key 整数键
     * @return 值(未找到返回空QByteArray)
     */
    QByteArray lookup(int key) const;

    /**
     * @brief 删除键
     * @param key 整数键
     */
    void remove(int key);

    /**
     * @brief 计算负载因子
     * @return 负载因子 [0.0, 1.0]
     */
    double loadFactor() const;

    /** @brief 当前元素数量 */
    int size() const { return m_size; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 重哈希完成信号 @param tableSize 新表大小 */
    void rehashed(int tableSize);

private:
    /**
     * @brief 哈希函数1
     * @param key 键
     * @param tableSize 表大小
     * @return 桶索引
     */
    static quint32 hash1(int key, int tableSize);

    /**
     * @brief 哈希函数2
     * @param key 键
     * @param tableSize 表大小
     * @return 桶索引
     */
    static quint32 hash2(int key, int tableSize);

    /**
     * @brief 扩容并重哈希所有元素
     */
    void rehash();

    /** @brief 桶条目 */
    struct Entry {
        int key = 0;                  ///< 键
        QByteArray value;             ///< 值
        bool occupied = false;        ///< 是否被占用
    };

    static constexpr int MAX_KICKS = 128; ///< 最大踢出次数

    int m_tableSize;                  ///< 单个表的大小
    int m_size;                       ///< 元素总数
    QVector<Entry> m_table1;          ///< 哈希表1
    QVector<Entry> m_table2;          ///< 哈希表2
    mutable Stats m_stats;            ///< 统计信息
    mutable double m_timeSumMs = 0.0; ///< 累计耗时(ms)
};

#endif // CUCKOOHASHTABLE_H
