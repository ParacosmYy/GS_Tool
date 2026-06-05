/**
 * @file FenwickTree2D.h
 * @brief 二维树状数组 — 2D Fenwick Tree (BIT) 支持区间求和与单点更新
 *
 * 功能: 提供二维平面上高效的点更新和前缀和查询，
 *       适用于矩阵区域求和、差分等场景。
 *
 * 协作: DataAggregator(聚合) / HistogramBuilder(直方图区域统计)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>
#include <tuple>

/**
 * @brief 二维树状数组 — 支持矩阵区域求和与点更新
 */
class FenwickTree2D : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates        = 0;   ///< 累计更新次数
        quint64 totalQueries        = 0;   ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        quint64 totalCellsUpdated   = 0;   ///< 累计更新单元格数
    };

    /**
     * @brief 构造函数
     * @param rows 行数(≥1)
     * @param cols 列数(≥1)
     * @param parent 父对象
     */
    explicit FenwickTree2D(int rows = 1, int cols = 1, QObject* parent = nullptr);

    /**
     * @brief 单点更新: 将(x,y)位置加上delta
     * @param x 行索引(0-based)
     * @param y 列索引(0-based)
     * @param delta 增量
     */
    void update(int x, int y, qint64 delta);

    /**
     * @brief 前缀和查询: (0,0)到(x,y)的矩形区域和
     * @param x 行索引(0-based, inclusive)
     * @param y 列索引(0-based, inclusive)
     * @return 前缀和
     */
    qint64 query(int x, int y) const;

    /**
     * @brief 区域和查询: (x1,y1)到(x2,y2)的矩形区域和
     * @param x1 左上行(0-based)
     * @param y1 左上列(0-based)
     * @param x2 右下行(0-based)
     * @param y2 右下列(0-based)
     * @return 区域和
     */
    qint64 rangeQuery(int x1, int y1, int x2, int y2) const;

    /**
     * @brief 获取原始矩阵中的值
     * @param x 行索引
     * @param y 列索引
     * @return 当前值
     */
    qint64 value(int x, int y) const;

    /**
     * @brief 批量更新
     * @param updates 更新列表 (x, y, delta)
     */
    void batchUpdate(const QVector<std::tuple<int, int, qint64>>& updates);

    /** @brief 获取行数 @return 行数 */
    int rows() const { return m_rows; }

    /** @brief 获取列数 @return 列数 */
    int cols() const { return m_cols; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 重置树(清零所有数据)
     */
    void clear();

signals:
    /** @brief 更新完成 @param x 行 @param y 列 @param delta 增量 */
    void cellUpdated(int x, int y, qint64 delta);

    /** @brief 批量更新完成 @param count 更新数量 */
    void batchUpdated(int count);

private:
    int m_rows;  ///< 行数
    int m_cols;  ///< 列数
    std::vector<std::vector<qint64>> m_tree;   ///< 树状数组(1-indexed)
    std::vector<std::vector<qint64>> m_data;   ///< 原始数据(0-indexed)

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
