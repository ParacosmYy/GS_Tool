/**
 * @file FenwickTree2D.cpp
 * @brief 二维树状数组实现
 */

#include "FenwickTree2D.h"
#include <QElapsedTimer>
#include <algorithm>

FenwickTree2D::FenwickTree2D(int rows, int cols, QObject* parent)
    : QObject(parent)
    , m_rows(rows)
    , m_cols(cols)
    , m_timeSum(0.0)
{
    m_tree1.resize(rows + 1, QVector<double>(cols + 1, 0.0));
    m_tree2.resize(rows + 1, QVector<double>(cols + 1, 0.0));
    m_tree3.resize(rows + 1, QVector<double>(cols + 1, 0.0));
    m_tree4.resize(rows + 1, QVector<double>(cols + 1, 0.0));
}

void FenwickTree2D::update(int r, int c, double delta)
{
    QElapsedTimer timer;
    timer.start();

    /* 1-indexed内部操作 */
    for (int i = r + 1; i <= m_rows; i += i & (-i))
        for (int j = c + 1; j <= m_cols; j += j & (-j))
            m_tree1[i][j] += delta;

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries +
                m_stats.totalRangeUpdates;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit updated(r, c, delta);
}

double FenwickTree2D::query(int r, int c) const
{
    double sum = 0.0;
    for (int i = r + 1; i > 0; i -= i & (-i))
        for (int j = c + 1; j > 0; j -= j & (-j))
            sum += m_tree1[i][j];
    return sum;
}

double FenwickTree2D::rangeQuery(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    double result = query(r2, c2);
    if (r1 > 0) result -= query(r1 - 1, c2);
    if (c1 > 0) result -= query(r2, c1 - 1);
    if (r1 > 0 && c1 > 0) result += query(r1 - 1, c1 - 1);

    const_cast<FenwickTree2D*>(this)->m_stats.totalQueries++;
    const_cast<FenwickTree2D*>(this)->m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries +
                m_stats.totalRangeUpdates;
    if (total > 0)
        const_cast<FenwickTree2D*>(this)->m_stats.avgProcessingTimeMs =
            m_timeSum / total;

    return result;
}

void FenwickTree2D::rangeUpdate(int r1, int c1, int r2, int c2, double delta)
{
    QElapsedTimer timer;
    timer.start();

    /* 使用4棵树状数组实现二维差分更新 */
    /* 对差分数组D[r][c]在(r1,c1)+delta, (r1,c2+1)-delta等 */
    internalUpdate(m_tree1, r1, c1, delta);
    internalUpdate(m_tree1, r1, c2 + 1, -delta);
    internalUpdate(m_tree1, r2 + 1, c1, -delta);
    internalUpdate(m_tree1, r2 + 1, c2 + 1, delta);

    internalUpdate(m_tree2, r1, c1, delta * (r1 - 1));
    internalUpdate(m_tree2, r1, c2 + 1, -delta * (r1 - 1));
    internalUpdate(m_tree2, r2 + 1, c1, -delta * r2);
    internalUpdate(m_tree2, r2 + 1, c2 + 1, delta * r2);

    internalUpdate(m_tree3, r1, c1, delta * (c1 - 1));
    internalUpdate(m_tree3, r1, c2 + 1, -delta * c2);
    internalUpdate(m_tree3, r2 + 1, c1, -delta * (c1 - 1));
    internalUpdate(m_tree3, r2 + 1, c2 + 1, delta * c2);

    internalUpdate(m_tree4, r1, c1, delta * (r1 - 1) * (c1 - 1));
    internalUpdate(m_tree4, r1, c2 + 1, -delta * (r1 - 1) * c2);
    internalUpdate(m_tree4, r2 + 1, c1, -delta * r2 * (c1 - 1));
    internalUpdate(m_tree4, r2 + 1, c2 + 1, delta * r2 * c2);

    m_stats.totalRangeUpdates++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries +
                m_stats.totalRangeUpdates;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

void FenwickTree2D::build(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 清空所有树 */
    for (int i = 0; i <= m_rows; ++i) {
        std::fill(m_tree1[i].begin(), m_tree1[i].end(), 0.0);
        std::fill(m_tree2[i].begin(), m_tree2[i].end(), 0.0);
        std::fill(m_tree3[i].begin(), m_tree3[i].end(), 0.0);
        std::fill(m_tree4[i].begin(), m_tree4[i].end(), 0.0);
    }

    /* 逐点插入 */
    for (int r = 0; r < qMin(data.size(), static_cast<int>(m_rows)); ++r) {
        for (int c = 0; c < qMin(data[r].size(), static_cast<int>(m_cols)); ++c) {
            internalUpdate(m_tree1, r, c, data[r][c]);
        }
    }

    m_timeSum += timer.elapsed();
}

void FenwickTree2D::internalUpdate(QVector<QVector<double>>& tree,
                                    int r, int c, double delta)
{
    if (r < 0 || c < 0) return;
    for (int i = r + 1; i <= m_rows && i > 0; i += i & (-i))
        for (int j = c + 1; j <= m_cols && j > 0; j += j & (-j))
            tree[i][j] += delta;
}

double FenwickTree2D::internalQuery(const QVector<QVector<double>>& tree,
                                     int r, int c) const
{
    double sum = 0.0;
    for (int i = r + 1; i > 0; i -= i & (-i))
        for (int j = c + 1; j > 0; j -= j & (-j))
            sum += tree[i][j];
    return sum;
}

void FenwickTree2D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
