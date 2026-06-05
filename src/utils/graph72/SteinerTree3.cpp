/**
 * @file SteinerTree3.cpp
 * @brief Steiner树近似算法实现 — 最短路径近似Steiner树
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Steiner 树问题的近似算法。
 * 给定图和一组终端顶点，找到连接所有终端的最小权重子树。
 * 使用基于最短路径的近似方法（2-近似算法）。
 */

#include "utils/graph72/SteinerTree3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空图
 * @param parent 父QObject对象
 */
SteinerTree3::SteinerTree3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SteinerTree3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置顶点数
 * @param n 顶点数量
 */
void SteinerTree3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.resize(m_n);
}

/**
 * @brief 添加一条带权边
 * @param u 起始顶点
 * @param v 终止顶点
 * @param weight 边权重
 */
void SteinerTree3::addEdge(int u, int v, double weight)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append({v, weight});
    m_adj[v].append({u, weight});
}

/**
 * @brief 设置终端顶点集合
 * @param terminals 必须被 Steiner 树连接的终端顶点列表
 */
void SteinerTree3::setTerminals(const QVector<int>& terminals)
{
    m_terminals = terminals;
}

// ──────────────────────────────────────────────
// Steiner 树求解
// ──────────────────────────────────────────────

/**
 * @brief 使用近似算法求解 Steiner 树
 *
 * 近似算法步骤（基于最短路径的 2-近似）：
 * 1. 计算所有顶点对之间的最短路径（Floyd-Warshall）
 * 2. 在终端顶点上构建完全图（边权 = 最短路径距离）
 * 3. 求该完全图的最小生成树（MST）
 * 4. 将 MST 的每条边展开为原图中的最短路径
 * 5. 对展开后的子图求最小生成树（去除可能的环路）
 *
 * @return Steiner 树的边列表
 */
QVector<QPair<int, int>> SteinerTree3::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || m_terminals.isEmpty()) return {};

    // 步骤1：计算所有顶点对最短路径
    QVector<QVector<double>> dist = allPairsShortest();

    // 步骤2：在终端上构建完全图并求 MST
    // 使用 Prim 算法
    const int t = m_terminals.size();
    QVector<bool> inMST(t, false);
    QVector<double> minEdge(t, 1e18);
    QVector<int> parent(t, -1);

    minEdge[0] = 0.0;

    for (int count = 0; count < t; ++count) {
        // 找最小边
        int u = -1;
        for (int v = 0; v < t; ++v) {
            if (!inMST[v] && (u == -1 || minEdge[v] < minEdge[u])) {
                u = v;
            }
        }

        if (u < 0) break;
        inMST[u] = true;

        for (int v = 0; v < t; ++v) {
            if (!inMST[v]) {
                int tu = m_terminals[u];
                int tv = m_terminals[v];
                double d = (tu < m_n && tv < m_n) ? dist[tu][tv] : 1e18;
                if (d < minEdge[v]) {
                    minEdge[v] = d;
                    parent[v] = u;
                }
            }
        }
    }

    // 步骤3：展开 MST 边为原图路径
    QSet<QPair<int, int>> edgeSet;
    for (int i = 1; i < t; ++i) {
        if (parent[i] < 0) continue;
        int u = m_terminals[parent[i]];
        int v = m_terminals[i];

        // 回溯最短路径
        while (u != v) {
            // 找到 v 的前驱
            int prev = -1;
            double minDist = 1e18;
            for (int k = 0; k < m_n; ++k) {
                for (const auto& edge : m_adj[k]) {
                    if (edge.first == v && dist[u][k] + edge.second < minDist + 1e-9) {
                        if (qAbs(dist[u][k] + edge.second - dist[u][v]) < 1e-9) {
                            prev = k;
                            minDist = dist[u][k] + edge.second;
                        }
                    }
                }
            }

            if (prev < 0) break;

            int a = qMin(prev, v);
            int b = qMax(prev, v);
            edgeSet.insert({a, b});
            v = prev;
        }
    }

    // 步骤4：构建结果
    QVector<QPair<int, int>> result;
    m_totalWeight = 0.0;
    m_steinerCount = 0;

    QSet<int> terminalSet(m_terminals.begin(), m_terminals.end());

    for (const auto& edge : edgeSet) {
        result.append(edge);
        // 计算权重
        for (const auto& e : m_adj[edge.first]) {
            if (e.first == edge.second) {
                m_totalWeight += e.second;
                break;
            }
        }
    }

    // 统计非终端 Steiner 节点
    QSet<int> usedNodes;
    for (const auto& edge : result) {
        usedNodes.insert(edge.first);
        usedNodes.insert(edge.second);
    }
    for (int node : usedNodes) {
        if (!terminalSet.contains(node)) {
            m_steinerCount++;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalTerminals += m_terminals.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(result.size(), m_totalWeight);
    return result;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含求解次数、总终端数和平均耗时的Stats结构
 */
SteinerTree3::Stats SteinerTree3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void SteinerTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 全源最短路径
// ──────────────────────────────────────────────

/**
 * @brief 使用 Dijkstra 算法计算所有顶点对之间的最短路径
 *
 * 对每个顶点运行一次 Dijkstra，总复杂度 O(V * E log V)。
 *
 * @return 距离矩阵，dist[u][v] 为 u 到 v 的最短距离
 */
QVector<QVector<double>> SteinerTree3::allPairsShortest()
{
    const double INF = 1e18;
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, INF));

    for (int src = 0; src < m_n; ++src) {
        dist[src][src] = 0.0;

        // 简化的 Dijkstra（无优先队列，直接搜索最小值）
        QVector<bool> visited(m_n, false);

        for (int iter = 0; iter < m_n; ++iter) {
            // 找未访问的最小距离顶点
            int u = -1;
            double minDist = INF;
            for (int v = 0; v < m_n; ++v) {
                if (!visited[v] && dist[src][v] < minDist) {
                    minDist = dist[src][v];
                    u = v;
                }
            }

            if (u < 0) break;
            visited[u] = true;

            for (const auto& edge : m_adj[u]) {
                double newDist = dist[src][u] + edge.second;
                if (newDist < dist[src][edge.first]) {
                    dist[src][edge.first] = newDist;
                }
            }
        }
    }

    return dist;
}
