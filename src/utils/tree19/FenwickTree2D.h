/**
 * @file FenwickTree2D.h
 * @brief 二维树状数组 — 2D前缀和/区间更新
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二维树状数组(Fenwick Tree 2D)
 * 支持单点更新+2D前缀和查询，或区间更新+单点查询
 */
class FenwickTree2D : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUpdates = 0;             ///< 累计更新次数
        int totalQueries = 0;             ///< 累计查询次数
        int totalRangeUpdates = 0;        ///< 累计区间更新次数
        double avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造函数
     * @param rows 行数
     * @param cols 列数
     * @param parent 父对象
     */
    explicit FenwickTree2D(int rows, int cols, QObject* parent = nullptr);

    /** @brief 单点更新 @param r 行 @param c 列 @param delta 增量 */
    void update(int r, int c, double delta);

    /** @brief 2D前缀和查询 sum[0..r][0..c] */
    double query(int r, int c) const;

    /** @brief 2D区间和查询 @param r1,r2 行范围 @param c1,c2 列范围 */
    double rangeQuery(int r1, int c1, int r2, int c2) const;

    /** @brief 区间更新: 矩形区域+delta(差分数组模式) */
    void rangeUpdate(int r1, int c1, int r2, int c2, double delta);

    /** @brief 批量构建(从2D数组) @param data 数据矩阵 */
    void build(const QVector<QVector<double>>& data);

    /** @brief 获取行数 */
    int rows() const { return m_rows; }
    /** @brief 获取列数 */
    int cols() const { return m_cols; }

    /** @brief 获取总元素数 */
    int size() const { return m_rows * m_cols; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 更新完成 @param r 行 @param c 列 @param delta 增量 */
    void updated(int r, int c, double delta);

private:
    int m_rows, m_cols;
    QVector<QVector<double>> m_tree1;   ///< 树状数组1
    QVector<QVector<double>> m_tree2;   ///< 树状数组2(差分用)
    QVector<QVector<double>> m_tree3;   ///< 树状数组3
    QVector<QVector<double>> m_tree4;   ///< 树状数组4

    /** @brief 内部更新辅助 */
    void internalUpdate(QVector<QVector<double>>& tree, int r, int c, double delta);

    /** @brief 内部前缀和查询 */
    double internalQuery(const QVector<QVector<double>>& tree, int r, int c) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
