/**
 * @file StreamingQuantile2.h
 * @brief 流式分位数估计器(GK算法增强版)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class StreamingQuantile2
 * @brief 流式分位数估计 — 基于Greenwald-Khanna算法的增强版
 *
 * 支持单遍扫描数据集的分位数查询，O(1/ε)空间复杂度。
 * 适用于实时监控、大规模数据分析等场景。
 */
class StreamingQuantile2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserted = 0;      /**< 总插入数 */
        int totalQueries = 0;       /**< 总查询数 */
        int currentTuples = 0;      /**< 当前元组数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param epsilon 误差容限(默认0.01即1%)
     * @param parent 父对象
     */
    explicit StreamingQuantile2(double epsilon = 0.01, QObject* parent = nullptr);

    /**
     * @brief 插入一个值
     * @param value 数值
     */
    void insert(double value);

    /**
     * @brief 查询指定分位数的值
     * @param phi 分位数(0~1)
     * @return 估计值
     */
    double query(double phi) const;

    /** @brief 批量插入 */
    void insertBatch(const QVector<double>& values);

    /** @brief 中位数 */
    double median() const;

    /** @brief 四分位数 */
    double q1() const;
    double q3() const;

    /** @brief 当前元组数 */
    int tupleCount() const;

    /** @brief 重置 */
    void reset();

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void valueInserted(double value, int totalInserted);

private:
    struct Tuple {
        double value;
        int gap;       /**< g: 该值与前一值之间的最小rank增量 */
        int delta;     /**< Δ: rank的最大可能误差 */
    };

    void compress();

    double m_epsilon;              /**< 误差容限 */
    QVector<Tuple> m_summary;      /**< 摘要结构 */
    int m_n;                       /**< 已处理元素数 */
    Stats m_stats;                 /**< 统计 */
    mutable double m_timeSum;      /**< 累计时间 */
};
