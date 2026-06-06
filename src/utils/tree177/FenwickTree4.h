/**
 * @file FenwickTree4.h
 * @brief 树状数组/二叉索引树(2D变体+区间更新查询+k阶统计) — Fenwick/BIT Tree with 2D Variant, Range Update/Query and k-th Order Statistic
 *
 * 功能: 实现Fenwick树(二叉索引树)，支持2D变体、区间更新/区间查询、
 *       k阶顺序统计量和逆序对计数。
 *
 * 协作: SegmentTree8(线段树) / BTree4(B树) / AvlTree3(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Fenwick树/二叉索引树(含2D变体)
 */
class FenwickTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalUpdates = 0;          ///< 累计更新次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        int treeSize = 0;                  ///< 树大小
        int dimensions = 1;                ///< 维度(1 or 2)
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit FenwickTree4(QObject *parent = nullptr);
    ~FenwickTree4() override;

    /** @brief 初始化1D Fenwick树 */
    void init1D(int size);

    /** @brief 从数组构建1D Fenwick树 */
    void build(const QVector<double>& data);

    /** @brief 初始化2D Fenwick树 */
    void init2D(int rows, int cols);

    /* ---- 1D operations ---- */

    /** @brief 单点更新: arr[idx] += delta */
    void update(int idx, double delta);

    /** @brief 前缀和[1..idx] */
    double prefixSum(int idx) const;

    /** @brief 区间和[l..r] */
    double rangeSum(int l, int r) const;

    /* ---- Range update / Range query (1D) ---- */

    /** @brief 区间更新: arr[l..r] += delta */
    void rangeUpdate(int l, int r, double delta);

    /** @brief 区间查询下的点值 */
    double pointQuery(int idx) const;

    /** @brief 区间查询下的区间和 */
    double rangeQuery(int l, int r) const;

    /* ---- 2D operations ---- */

    /** @brief 2D单点更新 */
    void update2D(int row, int col, double delta);

    /** @brief 2D前缀和 */
    double prefixSum2D(int row, int col) const;

    /** @brief 2D矩形和 */
    double rectSum(int r1, int c1, int r2, int c2) const;

    /* ---- k-th order statistic ---- */

    /** @brief 查找第k小的值(要求非负整数数据) */
    int findKth(int k) const;

    /** @brief 计算逆序对数 */
    qint64 countInversions(QVector<double> data) const;

    void clear();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updateCompleted(int idx, double value);
    void queryCompleted(double result);

private:
    /** @brief LSB操作 */
    static int lsb(int x) { return x & (-x); }

    int m_size = 0;
    int m_rows = 0;
    int m_cols = 0;

    QVector<double> m_tree;       ///< 1D BIT
    QVector<double> m_tree2;      ///< 1D BIT辅助(区间更新)
    QVector<QVector<double>> m_tree2D;  ///< 2D BIT

    /* Max value for k-th order statistic */
    int m_maxVal = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
