/**
 * @file FenwickTree2D2.h
 * @brief 二维树状数组(Binary Indexed Tree) — 支持2D前缀和查询与单点更新
 *
 * 功能: 实现二维Fenwick树(BIT)数据结构，支持高效的单点更新和
 *       二维前缀和/区间和查询。所有操作O(log(rows) * log(cols))时间复杂度。
 *       统计更新次数、查询次数和平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class FenwickTree2D2
 * @brief 二维树状数组 — 基于Fenwick/BIT的二维前缀和数据结构
 *
 * 使用二维差分思想，将一维Fenwick树扩展到二维平面。
 * 支持单点更新、前缀和查询、任意矩形区间和查询。
 * 适用于网格数据的频率统计、矩阵区域求和等场景。
 */
class FenwickTree2D2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        int totalUpdates = 0;       ///< 总更新次数
        int totalQueries = 0;       ///< 总查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数 — 初始化指定大小的二维树状数组
     * @param rows 行数(默认0)
     * @param cols 列数(默认0)
     * @param parent QObject父对象
     */
    explicit FenwickTree2D2(int rows = 0, int cols = 0, QObject* parent = nullptr);

    /**
     * @brief 单点更新 — 在(r, c)位置加上val
     * @param r 行索引(1-based)
     * @param c 列索引(1-based)
     * @param val 增量值
     */
    void update(int r, int c, double val);

    /**
     * @brief 前缀和查询 — 查询从(1,1)到(r,c)的矩形区域和
     * @param r 行索引(1-based)
     * @param c 列索引(1-based)
     * @return 前缀和
     */
    double query(int r, int c) const;

    /**
     * @brief 区间和查询 — 查询从(r1,c1)到(r2,c2)的矩形区域和
     * @param r1 左上角行索引(1-based)
     * @param c1 左上角列索引(1-based)
     * @param r2 右下角行索引(1-based)
     * @param c2 右下角列索引(1-based)
     * @return 区间和
     */
    double rangeQuery(int r1, int c1, int r2, int c2) const;

    /**
     * @brief 重置为指定大小 — 清空所有数据并重新分配
     * @param rows 新行数
     * @param cols 新列数
     */
    void reset(int rows, int cols);

    /**
     * @brief 清空所有数据 — 保持当前大小，所有位置归零
     */
    void clear();

    /** @brief 获取行数 */
    int rows() const { return m_rows; }

    /** @brief 获取列数 */
    int cols() const { return m_cols; }

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 更新完成信号 @param r 行 @param c 列 @param val 增量 */
    void updated(int r, int c, double val);

    /** @brief 查询完成信号 @param result 查询结果 */
    void queried(double result);

private:
    int m_rows = 0;                 ///< 行数
    int m_cols = 0;                 ///< 列数
    QVector<QVector<double>> m_tree; ///< 二维树状数组(1-based索引)
    mutable Stats m_stats;          ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时(ms)
    mutable int m_totalOps = 0;     ///< 总操作次数
};
