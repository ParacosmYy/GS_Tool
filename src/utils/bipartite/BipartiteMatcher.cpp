/**
 * @file BipartiteMatcher.cpp
 * @brief 二部图匹配引擎实现 — 简化匈牙利算法
 */

#include "utils/bipartite/BipartiteMatcher.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
BipartiteMatcher::BipartiteMatcher(QObject* parent)
    : QObject(parent)
    , m_totalCost(0.0)
    , m_timeSum(0.0)
{
}

/** @brief 执行最优匹配 @param cost 代价矩阵 @return 匹配对列表 */
QVector<QPair<int, int>> BipartiteMatcher::match(
    const QVector<QVector<double>>& cost)
{
    QElapsedTimer timer;
    timer.start();

    int n = cost.size();
    QVector<QPair<int, int>> result;

    if (n == 0) {
        emit matchingCompleted(0, 0.0);
        return result;
    }

    /* 统一为方阵: 取max(rows, cols)，空位填0 */
    int m = 0;
    for (const auto& row : cost) {
        m = qMax(m, row.size());
    }
    int sz = qMax(n, m);

    /* 构建扩展代价矩阵(转换为最小权匹配) */
    QVector<QVector<double>> c(sz, QVector<double>(sz, 0.0));
    for (int i = 0; i < n; ++i) {
        int cols = cost[i].size();
        for (int j = 0; j < cols; ++j) {
            c[i][j] = cost[i][j];
        }
    }

    /* 初始化势值与匹配数组 */
    m_rowMatch.assign(sz, -1);
    m_colMatch.assign(sz, -1);
    m_rowPot.assign(sz, 0.0);
    m_colPot.assign(sz, 0.0);

    /* 初始化行势值为各行最小值 */
    for (int i = 0; i < sz; ++i) {
        double minVal = c[i][0];
        for (int j = 1; j < sz; ++j) {
            minVal = qMin(minVal, c[i][j]);
        }
        m_rowPot[i] = minVal;
    }

    /* 逐行分配: 匈牙利算法主循环 */
    for (int u = 0; u < sz; ++u) {
        m_rowVisited.assign(sz, false);
        m_colVisited.assign(sz, false);

        while (!tryAssign(u, c)) {
            /* 计算最小松弛量 */
            double delta = std::numeric_limits<double>::max();
            for (int i = 0; i < sz; ++i) {
                if (!m_rowVisited[i]) continue;
                for (int j = 0; j < sz; ++j) {
                    if (m_colVisited[j]) continue;
                    double slack = c[i][j] - m_rowPot[i] - m_colPot[j];
                    delta = qMin(delta, slack);
                }
            }

            if (delta >= std::numeric_limits<double>::max() / 2) break;

            /* 更新势值 */
            for (int i = 0; i < sz; ++i) {
                if (m_rowVisited[i]) m_rowPot[i] += delta;
            }
            for (int j = 0; j < sz; ++j) {
                if (m_colVisited[j]) m_colPot[j] -= delta;
            }

            m_rowVisited.assign(sz, false);
            m_colVisited.assign(sz, false);
        }
    }

    /* 收集有效匹配 */
    m_totalCost = 0.0;
    for (int i = 0; i < n; ++i) {
        int j = m_rowMatch[i];
        if (j >= 0 && j < cost[i].size()) {
            result.append({i, j});
            m_totalCost += cost[i][j];
        }
    }

    /* 更新统计 */
    ++m_stats.totalMatches;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(result.size(), m_totalCost);
    return result;
}

/** @brief 尝试为行u分配列 @param u 行索引 @param cost 代价矩阵 @return 是否成功 */
bool BipartiteMatcher::tryAssign(int u, const QVector<QVector<double>>& cost)
{
    int sz = cost.size();
    m_rowVisited[u] = true;

    for (int v = 0; v < sz; ++v) {
        if (m_colVisited[v]) continue;
        double slack = cost[u][v] - m_rowPot[u] - m_colPot[v];
        if (qAbs(slack) < 1e-9) {
            m_colVisited[v] = true;
            if (m_colMatch[v] == -1 ||
                tryAssign(m_colMatch[v], cost)) {
                m_rowMatch[u] = v;
                m_colMatch[v] = u;
                return true;
            }
        }
    }
    return false;
}

/** @brief 重置统计 */
void BipartiteMatcher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_totalCost = 0.0;
}
