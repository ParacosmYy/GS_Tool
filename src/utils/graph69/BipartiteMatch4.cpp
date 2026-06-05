/**
 * @file BipartiteMatch4.cpp
 * @brief 二部图匹配实现 — 最大基数匹配 + 匈牙利最大权匹配
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现二部图的两种匹配算法：
 * 1. 基于 DFS 增广路径的最大基数匹配
 * 2. 基于匈牙利算法的最大权匹配
 */

#include "utils/graph69/BipartiteMatch4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空的二部图
 * @param parent 父QObject对象
 */
BipartiteMatch4::BipartiteMatch4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BipartiteMatch4"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置左侧顶点数
 * @param n 左侧顶点数
 */
void BipartiteMatch4::setLeftSize(int n)
{
    m_left = qMax(0, n);
    m_adj.resize(m_left);
}

/**
 * @brief 设置右侧顶点数
 * @param m 右侧顶点数
 */
void BipartiteMatch4::setRightSize(int m)
{
    m_right = qMax(0, m);
}

/**
 * @brief 添加一条带权边
 *
 * 在左侧顶点 u 和右侧顶点 v 之间添加一条权重为 weight 的边。
 *
 * @param u 左侧顶点索引（0-based）
 * @param v 右侧顶点索引（0-based）
 * @param weight 边权重
 */
void BipartiteMatch4::addEdge(int u, int v, double weight)
{
    if (u < 0 || u >= m_left || v < 0 || v >= m_right) return;
    m_adj[u].append({v, weight});
}

// ──────────────────────────────────────────────
// 最大基数匹配
// ──────────────────────────────────────────────

/**
 * @brief 使用 DFS 增广路径算法求最大基数匹配
 *
 * 对每个左侧顶点尝试匹配，使用 DFS 寻找增广路径。
 * 找到增广路径后翻转匹配关系，匹配数+1。
 *
 * @return 匹配边列表，每条边为 (左侧顶点, 右侧顶点)
 */
QVector<QPair<int, int>> BipartiteMatch4::maxMatching()
{
    QElapsedTimer timer;
    timer.start();

    // matchR[v] = 与右侧顶点v匹配的左侧顶点，-1表示未匹配
    QVector<int> matchR(m_right, -1);
    m_matchSize = 0;

    for (int u = 0; u < m_left; ++u) {
        QVector<bool> visited(m_right, false);
        if (dfs(u, visited, matchR)) {
            m_matchSize++;
        }
    }

    // 收集匹配结果
    QVector<QPair<int, int>> result;
    for (int v = 0; v < m_right; ++v) {
        if (matchR[v] >= 0) {
            result.append({matchR[v], v});
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_left + m_right;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(m_matchSize, 0.0);
    return result;
}

// ──────────────────────────────────────────────
// 最大权匹配（匈牙利算法）
// ──────────────────────────────────────────────

/**
 * @brief 使用匈牙利算法求最大权匹配
 *
 * 基于 Kuhn-Munkres 算法的简化实现。
 * 构建权重矩阵，通过势函数和增广路径搜索最大权和匹配。
 *
 * @return 匹配边列表，每条边为 (左侧顶点, 右侧顶点)
 */
QVector<QPair<int, int>> BipartiteMatch4::maxWeightMatching()
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMax(m_left, m_right);

    // 构建权重矩阵（补零使之成为方阵）
    QVector<QVector<double>> weight(n, QVector<double>(n, 0.0));
    for (int u = 0; u < m_left; ++u) {
        for (const auto& edge : m_adj[u]) {
            if (edge.first < n) {
                weight[u][edge.first] = edge.second;
            }
        }
    }

    // 势函数（标号）
    QVector<double> uLabel(n, 0.0);
    QVector<double> vLabel(n, 0.0);

    // 初始化左侧势为最大权重
    for (int u = 0; u < n; ++u) {
        double maxW = 0.0;
        for (int v = 0; v < n; ++v) {
            maxW = qMax(maxW, weight[u][v]);
        }
        uLabel[u] = maxW;
    }

    QVector<int> matchU(n, -1); // matchU[u] = 与u匹配的右侧顶点
    QVector<int> matchV(n, -1); // matchV[v] = 与v匹配的左侧顶点

    // 逐个匹配左侧顶点
    for (int uStart = 0; uStart < n; ++uStart) {
        QVector<bool> visitedU(n, false);
        QVector<bool> visitedV(n, false);
        QVector<double> slack(n, 1e18);
        QVector<int> slackV(n, -1);

        int currentU = uStart;
        visitedU[currentU] = true;

        bool found = false;
        while (!found) {
            // 更新 slack 值
            for (int v = 0; v < n; ++v) {
                if (!visitedV[v]) {
                    double gap = uLabel[currentU] + vLabel[v] - weight[currentU][v];
                    if (gap < slack[v]) {
                        slack[v] = gap;
                        slackV[v] = currentU;
                    }
                }
            }

            // 找最小 slack
            double minSlack = 1e18;
            int minV = -1;
            for (int v = 0; v < n; ++v) {
                if (!visitedV[v] && slack[v] < minSlack) {
                    minSlack = slack[v];
                    minV = v;
                }
            }

            if (minSlack > 1e-6) {
                // 更新势函数
                for (int u = 0; u < n; ++u) {
                    if (visitedU[u]) uLabel[u] -= minSlack;
                }
                for (int v = 0; v < n; ++v) {
                    if (visitedV[v]) {
                        vLabel[v] += minSlack;
                    } else {
                        slack[v] -= minSlack;
                    }
                }
            }

            // 将 minV 标记为已访问
            visitedV[minV] = true;

            if (matchV[minV] < 0) {
                // 找到增广路径，回溯更新匹配
                int v = minV;
                while (v >= 0) {
                    int u = slackV[v];
                    int nextV = matchU[u];
                    matchU[u] = v;
                    matchV[v] = u;
                    v = nextV;
                }
                found = true;
            } else {
                // 继续搜索
                currentU = matchV[minV];
                visitedU[currentU] = true;
            }
        }
    }

    // 收集结果
    QVector<QPair<int, int>> result;
    double totalWeight = 0.0;
    m_matchSize = 0;

    for (int u = 0; u < m_left; ++u) {
        if (matchU[u] >= 0 && matchU[u] < m_right && weight[u][matchU[u]] > 0.0) {
            result.append({u, matchU[u]});
            totalWeight += weight[u][matchU[u]];
            m_matchSize++;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_left + m_right;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(m_matchSize, totalWeight);
    return result;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含匹配次数、总顶点数和平均耗时的Stats结构
 */
BipartiteMatch4::Stats BipartiteMatch4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void BipartiteMatch4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — DFS增广
// ──────────────────────────────────────────────

/**
 * @brief DFS 搜索增广路径
 *
 * 从左侧顶点 u 出发，尝试找到一个未匹配的右侧顶点，
 * 或者为已匹配的右侧顶点重新分配新的左侧顶点。
 *
 * @param u 当前左侧顶点
 * @param visited 右侧顶点访问标记
 * @param matchR 右侧顶点的匹配状态
 * @return 是否找到增广路径
 */
bool BipartiteMatch4::dfs(int u, QVector<bool>& visited, QVector<int>& matchR)
{
    for (const auto& edge : m_adj[u]) {
        int v = edge.first;
        if (visited[v]) continue;
        visited[v] = true;

        if (matchR[v] < 0 || dfs(matchR[v], visited, matchR)) {
            matchR[v] = u;
            return true;
        }
    }
    return false;
}
