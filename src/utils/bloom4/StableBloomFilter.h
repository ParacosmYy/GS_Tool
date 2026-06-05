/**
 * @file StableBloomFilter.h
 * @brief Stable Bloom Filter — 支持元素过期的布隆过滤器
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class StableBloomFilter
 * @brief Stable布隆过滤器 — 支持元素自动过期衰减
 *
 * 每个计数器定期衰减，使旧元素自然过期。
 * 适用于数据流中的近似成员查询(如重复URL检测)。
 */
class StableBloomFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalAdds = 0;          /**< 总添加次数 */
        int totalQueries = 0;       /**< 总查询次数 */
        int totalFalsePositives = 0; /**< 估计误判次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param numCells 单元格数量(默认65536)
     * @param numHashes 哈希函数数量(默认3)
     * @param maxCounter 计数器最大值(默认3)
     * @param decayRate 衰减概率(默认0.05)
     * @param parent 父对象
     */
    explicit StableBloomFilter(int numCells = 65536, int numHashes = 3,
                                int maxCounter = 3, double decayRate = 0.05,
                                QObject* parent = nullptr);

    /** @brief 添加元素 */
    void add(const QByteArray& data);

    /** @brief 查询元素(可能存在) */
    bool contains(const QByteArray& data) const;

    /** @brief 添加字符串元素 */
    void addString(const QString& str);

    /** @brief 查询字符串 */
    bool containsString(const QString& str) const;

    /** @brief 手动触发一轮衰减 */
    void decay();

    /** @brief 清空过滤器 */
    void clear();

    /** @brief 估计误判率 */
    double estimatedFPR() const;

    /** @brief 获取单元格数量 */
    int cellCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 元素添加信号 */
    void elementAdded(const QByteArray& data);

private:
    quint64 hash(const QByteArray& data, int seed) const;

    QVector<quint8> m_cells;       /**< 计数器数组 */
    int m_numCells;                 /**< 单元格数 */
    int m_numHashes;                /**< 哈希函数数 */
    int m_maxCounter;               /**< 最大计数 */
    double m_decayRate;             /**< 衰减概率 */
    Stats m_stats;                  /**< 统计 */
    double m_timeSum;               /**< 累计时间 */
};
