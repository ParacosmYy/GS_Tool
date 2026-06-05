/**
 * @file FenwickTree.h
 * @brief 树状数组(BIT) — 前缀和/区间和/单点更新
 *
 * 功能: 树状数组(Binary Indexed Tree)，支持O(logN)前缀和查询、
 *       区间和查询、单点更新，统计查询/更新次数/耗时。
 */
#ifndef FENWICKTREE_H
#define FENWICKTREE_H

#include <QObject>
#include <QVector>

class FenwickTree : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalPrefixQueries = 0;
        quint64 totalRangeQueries = 0;
        quint64 totalUpdates = 0;
        int     treeSize = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree(QObject* parent = nullptr);

    /** @brief 初始化N个元素(初始值0) @param n 元素数 */
    void initialize(int n);

    /** @brief 从数组构建 @param data 输入数据 */
    void build(const QVector<double>& data);

    /** @brief 单点更新(增量) @param index 索引(0-based) @param delta 增量 */
    void update(int index, double delta);

    /** @brief 单点赋值 @param index 索引 @param value 新值 */
    void set(int index, double value);

    /** @brief 前缀和 [0, index] @param index 索引 @return 前缀和 */
    double prefixSum(int index) const;

    /** @brief 区间和 [left, right] @param left 左 @param right 右 @return 和 */
    double rangeSum(int left, int right) const;

    /** @brief 获取单点值 @param index 索引 @return 值 */
    double valueAt(int index) const;

    /** @brief 元素数 */
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updated(int index, double delta);
    void rangeQueried(int left, int right, double result);

private:
    int m_size;
    QVector<double> m_bit;      ///< 树状数组(1-indexed)
    QVector<double> m_original;  ///< 原始数组(0-indexed)

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // FENWICKTREE_H
