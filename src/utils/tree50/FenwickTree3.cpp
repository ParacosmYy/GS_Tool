/**
 * @file FenwickTree3.cpp
 * @brief Fenwick树3 — 二维范围+区间加+点查 实现
 *
 * 实现支持二维区间加法（矩形区域统一增量）和
 * 单点查询的 Fenwick 树。使用四个辅助树来支持
 * O(log R * log C) 的二维区间更新和点查询。
 *
 * 数学原理：
 *   对 [r1,c1]~[r2,c2] 加 delta 时，需要维护
 *   4 棵树分别对应展开后的 f(r,c) 多项式各项。
 *   get(i,j) = sum(i,j) 展开后由 4 棵树线性组合得到。
 */

#include "utils/tree50/FenwickTree3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化指定大小的二维 Fenwick 树
 * @param rows 行数
 * @param cols 列数
 * @param parent 父QObject
 */
FenwickTree3::FenwickTree3(int rows, int cols, QObject* parent)
    : QObject(parent)
    , m_rows(0)
    , m_cols(0)
{
    if (rows > 0 && cols > 0) {
        resize(rows, cols);
    }
}

/**
 * @brief 调整二维树的大小
 * @param rows 新行数
 * @param cols 新列数
 */
void FenwickTree3::resize(int rows, int cols)
{
    m_rows = qMax(0, rows);
    m_cols = qMax(0, cols);
    int r = m_rows + 1;
    int c = m_cols + 1;
    m_tree1.assign(r, QVector<double>(c, 0.0));
    m_tree2.assign(r, QVector<double>(c, 0.0));
    m_tree3.assign(r, QVector<double>(c, 0.0));
    m_tree4.assign(r, QVector<double>(c, 0.0));
}

/**
 * @brief 对辅助树执行单点增量更新
 *
 * 标准 Fenwick 树更新操作：从 (r,c) 开始，
 * 沿行和列方向跳转到后继节点。
 *
 * @param tree 目标辅助树
 * @param r 行索引（1-based）
 * @param c 列索引（1-based）
 * @param delta 增量值
 */
void FenwickTree3::internalAdd(QVector<QVector<double>>& tree, int r, int c, double delta)
{
    for (int i = r; i <= m_rows; i += i & (-i)) {
        for (int j = c; j <= m_cols; j += j & (-j)) {
            tree[i][j] += delta;
        }
    }
}

/**
 * @brief 对辅助树执行前缀和查询
 *
 * 标准 Fenwick 树查询操作：从 (r,c) 开始，
 * 沿行和列方向跳转到前驱节点并累加。
 *
 * @param tree 目标辅助树
 * @param r 行索引（1-based）
 * @param c 列索引（1-based）
 * @return 前缀和
 */
double FenwickTree3::internalSum(const QVector<QVector<double>>& tree, int r, int c) const
{
    double sum = 0.0;
    for (int i = r; i > 0; i -= i & (-i)) {
        for (int j = c; j > 0; j -= j & (-j)) {
            sum += tree[i][j];
        }
    }
    return sum;
}

/**
 * @brief 对单个位置执行增量更新
 *
 * 内部转换为区间更新 [r,c]~[r,c] 的特例。
 *
 * @param row 行索引（0-based）
 * @param col 列索引（0-based）
 * @param delta 增量值
 */
void FenwickTree3::add(int row, int col, double delta)
{
    addRange(row, col, row, col, delta);
}

/**
 * @brief 对矩形区域执行区间加法
 *
 * 使用 4 棵辅助树维护区间更新。
 * 对 [r1,c1]~[r2,c2] 加 delta 等价于：
 *   在 (r1,c1) 处加 +delta
 *   在 (r1,c2+1) 处加 -delta
 *   在 (r2+1,c1) 处加 -delta
 *   在 (r2+1,c2+1) 处加 +delta
 * 展开后的多项式由 4 棵树分别存储系数。
 *
 * @param r1 起始行（0-based）
 * @param c1 起始列（0-based）
 * @param r2 结束行（0-based）
 * @param c2 结束列（0-based）
 * @param delta 增量值
 */
void FenwickTree3::addRange(int r1, int c1, int r2, int c2, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows == 0 || m_cols == 0) return;
    if (r1 < 0 || c1 < 0 || r2 >= m_rows || c2 >= m_cols) return;
    if (r1 > r2 || c1 > c2) return;

    /* 转换为 1-based 索引 */
    int x1 = r1 + 1, y1 = c1 + 1;
    int x2 = r2 + 1, y2 = c2 + 1;

    /* 四树联合更新，展开多项式系数 */
    internalAdd(m_tree1, x1, y1, delta);
    internalAdd(m_tree1, x1, y2 + 1, -delta);
    internalAdd(m_tree1, x2 + 1, y1, -delta);
    internalAdd(m_tree1, x2 + 1, y2 + 1, delta);

    internalAdd(m_tree2, x1, y1, delta * (1.0 - y1));
    internalAdd(m_tree2, x1, y2 + 1, delta * (y2));
    internalAdd(m_tree2, x2 + 1, y1, -delta * (1.0 - y1));
    internalAdd(m_tree2, x2 + 1, y2 + 1, -delta * (y2));

    internalAdd(m_tree3, x1, y1, delta * (1.0 - x1));
    internalAdd(m_tree3, x1, y2 + 1, -delta * (1.0 - x1));
    internalAdd(m_tree3, x2 + 1, y1, delta * (x2));
    internalAdd(m_tree3, x2 + 1, y2 + 1, -delta * (x2));

    internalAdd(m_tree4, x1, y1, delta * (1.0 - x1) * (1.0 - y1));
    internalAdd(m_tree4, x1, y2 + 1, -delta * (1.0 - x1) * y2);
    internalAdd(m_tree4, x2 + 1, y1, -delta * x2 * (1.0 - y1));
    internalAdd(m_tree4, x2 + 1, y2 + 1, delta * x2 * y2);

    /* 统计更新 */
    m_stats.totalUpdates++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalUpdates + m_stats.totalQueries + m_stats.totalRangeQueries;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    emit updated(r1, c1, delta);
}

/**
 * @brief 计算 (0,0) 到 (row,col) 的前缀和
 *
 * 由 4 棵辅助树组合计算：
 *   S(r,c) = tree1(r,c)*r*c + tree2(r,c)*r + tree3(r,c)*c + tree4(r,c)
 *
 * @param row 行索引（0-based）
 * @param col 列索引（0-based）
 * @return 前缀和值
 */
double FenwickTree3::sum(int row, int col) const
{
    if (m_rows == 0 || m_cols == 0) return 0.0;
    if (row < 0 || col < 0) return 0.0;

    int r = qMin(row + 1, m_rows);
    int c = qMin(col + 1, m_cols);

    double s1 = internalSum(m_tree1, r, c) * r * c;
    double s2 = internalSum(m_tree2, r, c) * r;
    double s3 = internalSum(m_tree3, r, c) * c;
    double s4 = internalSum(m_tree4, r, c);

    return s1 + s2 + s3 + s4;
}

/**
 * @brief 计算矩形区域 [r1,c1]~[r2,c2] 的范围和
 *
 * 使用二维容斥原理：
 *   rangeSum = S(r2,c2) - S(r1-1,c2) - S(r2,c1-1) + S(r1-1,c1-1)
 *
 * @param r1 起始行（0-based）
 * @param c1 起始列（0-based）
 * @param r2 结束行（0-based）
 * @param c2 结束列（0-based）
 * @return 范围和
 */
double FenwickTree3::rangeSum(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    double result = sum(r2, c2)
                  - (r1 > 0 ? sum(r1 - 1, c2) : 0.0)
                  - (c1 > 0 ? sum(r2, c1 - 1) : 0.0)
                  + (r1 > 0 && c1 > 0 ? sum(r1 - 1, c1 - 1) : 0.0);

    /* 统计更新（const_cast 仅用于统计） */
    const_cast<FenwickTree3*>(this)->m_stats.totalRangeQueries++;
    qint64 elapsed = timer.elapsed();
    const_cast<FenwickTree3*>(this)->m_timeSum += elapsed;
    int totalOps = m_stats.totalUpdates + m_stats.totalQueries + m_stats.totalRangeQueries;
    if (totalOps > 0) {
        const_cast<FenwickTree3*>(this)->m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    return result;
}

/**
 * @brief 获取指定位置的单点值
 *
 * 由于本树设计为"区间加+点查"模式，
 * get(i,j) 等价于 sum(i,j) - sum(i-1,j) - sum(i,j-1) + sum(i-1,j-1)。
 * 但在区间加法模型下，sum(i,j) 本身就是该点的值。
 *
 * @param row 行索引（0-based）
 * @param col 列索引（0-based）
 * @return 该位置的当前值
 */
double FenwickTree3::get(int row, int col) const
{
    QElapsedTimer timer;
    timer.start();

    double val = sum(row, col)
               - (row > 0 ? sum(row - 1, col) : 0.0)
               - (col > 0 ? sum(row, col - 1) : 0.0)
               + (row > 0 && col > 0 ? sum(row - 1, col - 1) : 0.0);

    const_cast<FenwickTree3*>(this)->m_stats.totalQueries++;
    qint64 elapsed = timer.elapsed();
    const_cast<FenwickTree3*>(this)->m_timeSum += elapsed;
    int totalOps = m_stats.totalUpdates + m_stats.totalQueries + m_stats.totalRangeQueries;
    if (totalOps > 0) {
        const_cast<FenwickTree3*>(this)->m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    return val;
}

/**
 * @brief 设置指定位置的值（先查询当前值，再补偿差值）
 * @param row 行索引（0-based）
 * @param col 列索引（0-based）
 * @param value 目标值
 */
void FenwickTree3::set(int row, int col, double value)
{
    double current = get(row, col);
    double delta = value - current;
    if (!qFuzzyIsNull(delta)) {
        add(row, col, delta);
    }
}

/**
 * @brief 清空所有数据，重置为初始状态
 */
void FenwickTree3::clear()
{
    int r = m_rows + 1;
    int c = m_cols + 1;
    for (int i = 0; i < r; ++i) {
        m_tree1[i].fill(0.0);
        m_tree2[i].fill(0.0);
        m_tree3[i].fill(0.0);
        m_tree4[i].fill(0.0);
    }
}

/**
 * @brief 重置所有统计数据
 */
void FenwickTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
