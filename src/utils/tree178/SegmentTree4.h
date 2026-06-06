/**
 * @file SegmentTree4.h
 * @brief 线段树(懒惰传播+区间乘加查询+可持久化版本) — Segment Tree with Lazy Propagation, Range Multiply/Add Queries and Persistent Version
 *
 * 功能: 实现线段树，支持懒惰传播、区间乘法/加法混合操作、
 *       区间查询和可持久化版本管理。
 *
 * 协作: FenwickTree4(树状数组) / BTree4(B树) / RedBlackTree5(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 线段树(懒惰传播+持久化)
 */
class SegmentTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalQueries = 0;
        int treeSize = 0;
        int numVersions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree4(QObject *parent = nullptr);
    ~SegmentTree4() override;

    /** @brief 从数组构建线段树 */
    void build(const QVector<double>& data);

    /** @brief 区间加法: [l,r] += val */
    void rangeAdd(int l, int r, double val);

    /** @brief 区间乘法: [l,r] *= val */
    void rangeMultiply(int l, int r, double val);

    /** @brief 区间求和查询: sum([l,r]) */
    double rangeQuery(int l, int r);

    /** @brief 点查询 */
    double pointQuery(int idx);

    /** @brief 保存当前版本(持久化) */
    int saveVersion();

    /** @brief 恢复到指定版本 */
    void restoreVersion(int versionId);

    /** @brief 获取版本数 */
    int versionCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updateCompleted(int l, int r, double value);
    void queryCompleted(double result);

private:
    /** @brief 懒惰标记: 乘法和加法 */
    struct LazyTag {
        double mul = 1.0;  ///< 乘法标记
        double add = 0.0;  ///< 加法标记
    };

    int m_size = 0;
    QVector<double> m_tree;      ///< Segment tree values (sum)
    QVector<LazyTag> m_lazy;     ///< Lazy propagation tags
    QVector<QVector<double>> m_versions; ///< Persistent versions of data

    void pushDown(int node, int left, int right);
    void pushUp(int node);
    void buildImpl(int node, int left, int right, const QVector<double>& data);
    void rangeAddImpl(int node, int l, int r, int ql, int qr, double val);
    void rangeMulImpl(int node, int l, int r, int ql, int qr, double val);
    double queryImpl(int node, int l, int r, int ql, int qr);

    Stats m_stats;
    double m_timeSum = 0.0;
};
