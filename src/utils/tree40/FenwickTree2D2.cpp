/**
 * @file FenwickTree2D2.cpp
 * @brief 二维树状数组实现 — 高效2D前缀和与区间查询
 *
 * 实现二维Fenwick树(Binary Indexed Tree 2D):
 * - 单点更新: O(log(rows) * log(cols))
 * - 前缀和查询: O(log(rows) * log(cols))
 * - 区间和查询: 利用容斥原理 O(log(rows) * log(cols))
 *
 * 原理:
 * 将一维Fenwick树嵌套为二维结构，外层树管理行方向，
 * 内层树管理列方向。更新/查询时双层循环遍历，
 * 每层使用经典的i += i&(-i) / i -= i&(-i)跳转。
 *
 * 统计更新次数、查询次数、平均耗时。
 */

#include "utils/tree40/FenwickTree2D2.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 创建指定大小的二维树状数组
 *
 * 内部使用1-based索引，分配 (rows+1) x (cols+1) 的二维数组，
 * 第0行和第0列不使用，简化边界处理。
 *
 * @param rows 行数(默认0)
 * @param cols 列数(默认0)
 * @param parent QObject父对象
 */
FenwickTree2D2::FenwickTree2D2(int rows, int cols, QObject* parent)
    : QObject(parent)
    , m_rows(qMax(0, rows))
    , m_cols(qMax(0, cols))
    , m_timeSum(0.0)
    , m_totalOps(0)
{
    if (m_rows > 0 && m_cols > 0) {
        /* 1-based索引: 分配rows+1行, cols+1列 */
        m_tree.resize(m_rows + 1);
        for (int i = 0; i <= m_rows; ++i) {
            m_tree[i].resize(m_cols + 1, 0.0);
        }
    }
}

/**
 * @brief 单点更新 — 在位置(r, c)加上增量val
 *
 * 更新流程(双层Fenwick更新):
 * 外层: i从r向上跳转 i += i & (-i)
 * 内层: j从c向上跳转 j += j & (-j)
 *
 * @param r 行索引(1-based，范围[1, rows])
 * @param c 列索引(1-based，范围[1, cols])
 * @param val 增量值(可为负数)
 */
void FenwickTree2D2::update(int r, int c, double val)
{
    QElapsedTimer timer;
    timer.start();

    /* 边界检查 */
    if (r < 1 || r > m_rows || c < 1 || c > m_cols) {
        m_timeSum += timer.elapsed();
        m_stats.totalUpdates++;
        m_totalOps++;
        m_stats.avgProcessingTimeMs =
            (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;
        emit updated(r, c, val);
        return;
    }

    /* 双层Fenwick更新 */
    for (int i = r; i <= m_rows; i += i & (-i)) {
        for (int j = c; j <= m_cols; j += j & (-j)) {
            m_tree[i][j] += val;
        }
    }

    /* 更新统计信息 */
    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit updated(r, c, val);
}

/**
 * @brief 前缀和查询 — 从(1,1)到(r,c)的矩形区域元素和
 *
 * 查询流程(双层Fenwick查询):
 * 外层: i从r向下跳转 i -= i & (-i)
 * 内层: j从c向下跳转 j -= j & (-j)
 * 累加所有经过的节点值。
 *
 * @param r 行索引(1-based)
 * @param c 列索引(1-based)
 * @return 前缀和 S(r,c) = sum of all (i,j) where 1<=i<=r, 1<=j<=c
 */
double FenwickTree2D2::query(int r, int c) const
{
    QElapsedTimer timer;
    timer.start();

    double sum = 0.0;

    /* 边界保护 */
    r = qBound(0, r, m_rows);
    c = qBound(0, c, m_cols);

    /* 双层Fenwick查询 */
    for (int i = r; i > 0; i -= i & (-i)) {
        for (int j = c; j > 0; j -= j & (-j)) {
            sum += m_tree[i][j];
        }
    }

    /* 更新统计信息 */
    const_cast<FenwickTree2D2*>(this)->m_stats.totalQueries++;
    const_cast<FenwickTree2D2*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree2D2*>(this)->m_totalOps++;
    const_cast<FenwickTree2D2*>(this)->m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit const_cast<FenwickTree2D2*>(this)->queried(sum);
    return sum;
}

/**
 * @brief 区间和查询 — 任意矩形区域的元素和
 *
 * 利用二维容斥原理(包含-排除):
 * S(r1,c1,r2,c2) = S(r2,c2) - S(r1-1,c2) - S(r2,c1-1) + S(r1-1,c1-1)
 *
 * @param r1 左上角行索引(1-based)
 * @param c1 左上角列索引(1-based)
 * @param r2 右下角行索引(1-based)
 * @param c2 右下角列索引(1-based)
 * @return 区间和
 */
double FenwickTree2D2::rangeQuery(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    /* 边界检查与规范化 */
    r1 = qBound(1, r1, m_rows);
    c1 = qBound(1, c1, m_cols);
    r2 = qBound(1, r2, m_rows);
    c2 = qBound(1, c2, m_cols);

    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);

    /* 容斥原理: S(r2,c2) - S(r1-1,c2) - S(r2,c1-1) + S(r1-1,c1-1) */
    double result = queryInternal(r2, c2)
                  - queryInternal(r1 - 1, c2)
                  - queryInternal(r2, c1 - 1)
                  + queryInternal(r1 - 1, c1 - 1);

    /* 更新统计信息 */
    const_cast<FenwickTree2D2*>(this)->m_stats.totalQueries++;
    const_cast<FenwickTree2D2*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree2D2*>(this)->m_totalOps++;
    const_cast<FenwickTree2D2*>(this)->m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit const_cast<FenwickTree2D2*>(this)->queried(result);
    return result;
}

/**
 * @brief 重置为新的尺寸 — 清空所有数据并重新分配内存
 * @param rows 新行数
 * @param cols 新列数
 */
void FenwickTree2D2::reset(int rows, int cols)
{
    m_rows = qMax(0, rows);
    m_cols = qMax(0, cols);

    m_tree.clear();

    if (m_rows > 0 && m_cols > 0) {
        m_tree.resize(m_rows + 1);
        for (int i = 0; i <= m_rows; ++i) {
            m_tree[i].resize(m_cols + 1, 0.0);
        }
    }
}

/**
 * @brief 清空所有数据 — 保持当前尺寸，所有位置归零
 */
void FenwickTree2D2::clear()
{
    for (int i = 0; i <= m_rows; ++i) {
        for (int j = 0; j <= m_cols; ++j) {
            m_tree[i][j] = 0.0;
        }
    }
}

/**
 * @brief 重置统计信息
 */
void FenwickTree2D2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_totalOps = 0;
}

/* ===== 私有方法实现 ===== */

/**
 * @brief 内部前缀和查询 — 不更新统计信息，用于rangeQuery的容斥计算
 * @param r 行索引(可为0)
 * @param c 列索引(可为0)
 * @return 前缀和
 */
double FenwickTree2D2::queryInternal(int r, int c) const
{
    if (r <= 0 || c <= 0) return 0.0;

    r = qMin(r, m_rows);
    c = qMin(c, m_cols);

    double sum = 0.0;
    for (int i = r; i > 0; i -= i & (-i)) {
        for (int j = c; j > 0; j -= j & (-j)) {
            sum += m_tree[i][j];
        }
    }
    return sum;
}
