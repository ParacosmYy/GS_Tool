/**
 * @file RangeMinimumQuery.h
 * @brief 区间最小值查询(Sparse Table) — O(1)查询, O(n log n)预处理
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class RangeMinimumQuery
 * @brief 基于Sparse Table的RMQ数据结构
 *
 * 预处理 O(n log n)，查询 O(1)。
 * 适用于静态数据（构建后不修改）上的频繁区间最小值查询。
 * 同时支持查询最小值和最小值的原始索引。
 */
class RangeMinimumQuery : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalQueries = 0;          ///< 总查询次数
        quint64 totalBuilds = 0;           ///< 总构建次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit RangeMinimumQuery(QObject* parent = nullptr);

    /**
     * @brief 构建Sparse Table
     * @param data 输入数据数组
     *
     * 预处理时间 O(n log n)，空间 O(n log n)
     */
    void build(const QVector<double>& data);

    /**
     * @brief 查询区间 [left, right] 的最小值
     * @param left 左边界（含）
     * @param right 右边界（含）
     * @return 区间最小值
     */
    double query(int left, int right) const;

    /**
     * @brief 查询区间 [left, right] 最小值的原始索引
     * @param left 左边界（含）
     * @param right 右边界（含）
     * @return 最小值在原数组中的索引
     */
    int queryIndex(int left, int right) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 @param n 数据长度 */
    void buildCompleted(int n);

private:
    mutable Stats  m_stats;
    mutable double m_timeSum = 0.0;   ///< 累计处理时间

    QVector<double>           m_data; ///< 原始数据
    QVector<QVector<int>>     m_sparseTable; ///< Sparse Table (存索引)
    QVector<int>              m_logTable;     ///< floor(log2(i)) 查找表

    /** @brief 构建log查找表 */
    void buildLogTable(int n);
};
