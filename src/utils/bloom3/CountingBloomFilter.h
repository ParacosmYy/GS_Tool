/**
 * @file CountingBloomFilter.h
 * @brief 计数型布隆过滤器 — 频率估计与元素删除
 *
 * 功能: 在标准布隆过滤器基础上使用计数器数组，支持
 *       元素频率估计、删除操作。适用于数据流频率统计、
 *       重复检测、缓存淘汰策略。
 *
 * 协作: BloomFilter(存在性检测) / DataAggregator(聚合统计)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QString>
#include <QElapsedTimer>

/**
 * @brief 计数型布隆过滤器 — 频率估计与可删除
 */
class FrequencyBloomFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalAdds = 0;              ///< 累计添加次数
        quint64 totalQueries = 0;           ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param expectedItems 预期元素数(默认10000)
     * @param fpRate 期望误判率(默认0.01)
     * @param parent 父对象
     */
    explicit FrequencyBloomFilter(quint64 expectedItems = 10000,
                                 double fpRate = 0.01,
                                 QObject* parent = nullptr);

    /**
     * @brief 添加元素
     * @param item 数据项
     */
    void add(const QByteArray& item);

    /**
     * @brief 添加元素(QString重载)
     * @param item 字符串数据项
     */
    void add(const QString& item);

    /**
     * @brief 查询元素估计频率
     * @param item 数据项
     * @return 估计出现次数
     */
    int count(const QByteArray& item) const;

    /**
     * @brief 查询元素估计频率(QString重载)
     * @param item 字符串数据项
     * @return 估计出现次数
     */
    int count(const QString& item) const;

    /**
     * @brief 移除元素(计数器减1)
     * @param item 数据项
     * @return 是否成功移除(元素存在)
     */
    bool remove(const QByteArray& item);

    /**
     * @brief 检查元素是否存在(计数>0)
     * @param item 数据项
     * @return 是否可能存在
     */
    bool contains(const QByteArray& item) const;

    /** @brief 清空过滤器 */
    void clear();

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 元素添加信号 @param totalAdds 累计添加数 */
    void elementAdded(quint64 totalAdds);

    /** @brief 元素移除信号 @param success 是否成功 */
    void elementRemoved(bool success);

private:
    /**
     * @brief 计算第n个哈希值
     * @param data 数据
     * @param n 哈希序号
     * @return 哈希位置
     */
    quint32 hashN(const QByteArray& data, int n) const;

    /**
     * @brief MurmurHash3变体
     * @param data 数据
     * @param seed 种子
     * @return 哈希值
     */
    quint32 murmurHash(const QByteArray& data, quint32 seed) const;

    QVector<quint32> m_counters;     ///< 计数器数组
    quint64 m_counterCount;          ///< 计数器数量
    int m_hashCount;                 ///< 哈希函数数量
    quint64 m_expectedItems;         ///< 预期元素数
    double m_fpRate;                 ///< 期望误判率

    mutable QElapsedTimer m_timer;   ///< 计时器
    mutable double m_timeSum;        ///< 累计耗时
    mutable Stats m_stats;
};
