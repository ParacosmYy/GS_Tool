/**
 * @file FenwickTree2D.cpp
 * @brief 二维树状数组实现 — 区间求和与单点更新
 */

#include "utils/fenwick2/FenwickTree2D.h"

#include <QElapsedTimer>

#include <algorithm>

/** @brief 构造函数 @param rows 行数 @param cols 列数 @param parent 父对象 */
FenwickTree2D::FenwickTree2D(int rows, int cols, QObject* parent)
    : QObject(parent)
    , m_rows(std::max(1, rows))
    , m_cols(std::max(1, cols))
    , m_tree(m_rows + 1, std::vector<qint64>(m_cols + 1, 0))
    , m_data(m_rows, std::vector<qint64>(m_cols, 0))
{
}

/** @brief 单点更新 @param x 行 @param y 列 @param delta 增量 */
void FenwickTree2D::update(int x, int y, qint64 delta)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;

    QElapsedTimer timer;
    timer.start();

    m_data[x][y] += delta;

    /* BIT是1-indexed，内部从(x+1, y+1)开始 */
    for (int i = x + 1; i <= m_rows; i += i & (-i)) {
        for (int j = y + 1; j <= m_cols; j += j & (-j)) {
            m_tree[i][j] += delta;
        }
    }

    ++m_stats.totalUpdates;
    ++m_stats.totalCellsUpdated;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit cellUpdated(x, y, delta);
}

/** @brief 前缀和查询 @param x 行 @param y 列 @return 前缀和 */
qint64 FenwickTree2D::query(int x, int y) const
{
    if (x < 0 || y < 0) return 0;
    x = std::min(x, m_rows - 1);
    y = std::min(y, m_cols - 1);

    QElapsedTimer timer;
    timer.start();

    qint64 sum = 0;
    for (int i = x + 1; i > 0; i -= i & (-i)) {
        for (int j = y + 1; j > 0; j -= j & (-j)) {
            sum += m_tree[i][j];
        }
    }

    ++m_stats.totalQueries;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    return sum;
}

/** @brief 区域和查询 @param x1 左上行 @param y1 左上列 @param x2 右下行 @param y2 右下列 @return 区域和 */
qint64 FenwickTree2D::rangeQuery(int x1, int y1, int x2, int y2) const
{
    /* 容斥原理: S(x2,y2) - S(x1-1,y2) - S(x2,y1-1) + S(x1-1,y1-1) */
    qint64 s22 = query(x2, y2);
    qint64 s02 = (x1 > 0) ? query(x1 - 1, y2) : 0;
    qint64 s20 = (y1 > 0) ? query(x2, y1 - 1) : 0;
    qint64 s00 = (x1 > 0 && y1 > 0) ? query(x1 - 1, y1 - 1) : 0;
    return s22 - s02 - s20 + s00;
}

/** @brief 获取原始值 @param x 行 @param y 列 @return 当前值 */
qint64 FenwickTree2D::value(int x, int y) const
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return 0;
    return m_data[x][y];
}

/** @brief 批量更新 @param updates 更新列表 */
void FenwickTree2D::batchUpdate(const QVector<std::tuple<int, int, qint64>>& updates)
{
    QElapsedTimer timer;
    timer.start();

    for (const auto& u : updates) {
        int x = std::get<0>(u);
        int y = std::get<1>(u);
        qint64 delta = std::get<2>(u);
        if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) continue;

        m_data[x][y] += delta;
        for (int i = x + 1; i <= m_rows; i += i & (-i)) {
            for (int j = y + 1; j <= m_cols; j += j & (-j)) {
                m_tree[i][j] += delta;
            }
        }
        ++m_stats.totalCellsUpdated;
    }

    m_stats.totalUpdates += static_cast<quint64>(updates.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit batchUpdated(updates.size());
}

/** @brief 重置树 */
void FenwickTree2D::clear()
{
    for (int i = 0; i <= m_rows; ++i) {
        std::fill(m_tree[i].begin(), m_tree[i].end(), 0);
    }
    for (int i = 0; i < m_rows; ++i) {
        std::fill(m_data[i].begin(), m_data[i].end(), 0);
    }
}

/** @brief 重置统计 */
void FenwickTree2D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
