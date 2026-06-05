/**
 * @file ChinesePostman2.cpp
 * @brief 中国邮路问题求解实现 — 欧拉回路 + 奇度顶点匹配
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 求解中国邮路问题：找到经过图中所有边至少一次的最短闭合回路。
 * 对于欧拉图，直接找到欧拉回路即可；
 * 对于非欧拉图，先用最短路径匹配奇度顶点，再找欧拉回路。
 */

#include "utils/graph70/ChinesePostman2.h"

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
ChinesePostman2::ChinesePostman2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ChinesePostman2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置图的有向/无向属性
 * @param directed true 为有向图，false 为无向图
 */
void ChinesePostman2::setDirected(bool directed)
{
    m_directed = directed;
}

/**
 * @brief 添加一条边
 *
 * 自动扩展图的顶点范围。
 *
 * @param u 起始顶点
 * @param v 终止顶点
 * @param weight 边权重
 */
void ChinesePostman2::addEdge(int u, int v, double weight)
{
    int maxV = qMax(u, v) + 1;
    if (maxV > m_n) {
        m_adj.resize(maxV);
        for (auto& list : m_adj) {
            // 保持已有边
        }
        m_n = maxV;
    }
    m_adj[u].append({v, weight});

    if (!m_directed) {
        // 确保双向对称
        if (v + 1 > m_n) {
            m_adj.resize(v + 1);
            m_n = v + 1;
        }
        m_adj[v].append({u, weight});
    }
}

// ──────────────────────────────────────────────
// 求解中国邮路
// ──────────────────────────────────────────────

/**
 * @brief 求解中国邮路问题
 *
 * 算法步骤：
 * 1. 检查图连通性
 * 2. 检查是否为欧拉图（所有顶点度数为偶数）
 * 3. 如果不是欧拉图，找到所有奇度顶点并配对
 * 4. 在配对顶点之间复制最短路径上的边
 * 5. 找欧拉回路
 *
 * @return 邮路经过的顶点序列
 */
QVector<int> ChinesePostman2::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        return {};
    }

    // 步骤1：计算各顶点度数
    QVector<int> degree(m_n, 0);
    int totalEdges = 0;
    for (int u = 0; u < m_n; ++u) {
        degree[u] = m_adj[u].size();
        totalEdges += m_adj[u].size();
    }
    if (!m_directed) totalEdges /= 2;

    // 步骤2：检查奇度顶点
    QVector<int> oddVertices;
    for (int v = 0; v < m_n; ++v) {
        if (degree[v] % 2 != 0) {
            oddVertices.append(v);
        }
    }

    m_eulerian = oddVertices.isEmpty();

    // 步骤3：如果有奇度顶点，执行匹配
    if (!m_eulerian) {
        matchOddVertices();
    }

    // 步骤4：找欧拉回路
    QVector<int> tour = findEulerTour();

    // 计算总代价
    m_cost = 0.0;
    for (int i = 0; i < static_cast<int>(tour.size()) - 1; ++i) {
        int u = tour[i];
        int v = tour[i + 1];
        for (const auto& edge : m_adj[u]) {
            if (edge.first == v) {
                m_cost += edge.second;
                break;
            }
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalEdges += totalEdges;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(totalEdges, m_cost);
    return tour;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含求解次数、总边数和平均耗时的Stats结构
 */
ChinesePostman2::Stats ChinesePostman2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ChinesePostman2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 欧拉回路
// ──────────────────────────────────────────────

/**
 * @brief 使用 Hierholzer 算法找欧拉回路
 *
 * 从任意有边的顶点出发，沿未访问的边前进，
 * 直到回到起点形成回路。然后将回路插入到其他位置。
 *
 * @return 欧拉回路的顶点序列
 */
QVector<int> ChinesePostman2::findEulerTour()
{
    if (m_n == 0) return {};

    // 复制邻接表（使用边索引跟踪已访问）
    QVector<QVector<QPair<int, double>>> adjCopy = m_adj;
    QVector<QVector<bool>> used(m_n);
    for (int u = 0; u < m_n; ++u) {
        used[u].resize(adjCopy[u].size(), false);
    }

    // 找起始顶点（有边的第一个顶点）
    int start = 0;
    for (int v = 0; v < m_n; ++v) {
        if (!adjCopy[v].isEmpty()) {
            start = v;
            break;
        }
    }

    // Hierholzer 算法
    QVector<int> stack;
    QVector<int> circuit;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool foundEdge = false;

        for (int i = 0; i < adjCopy[v].size(); ++i) {
            if (!used[v][i]) {
                int u = adjCopy[v][i].first;
                used[v][i] = true;

                // 无向图：标记反向边
                if (!m_directed) {
                    for (int j = 0; j < adjCopy[u].size(); ++j) {
                        if (!used[u][j] && adjCopy[u][j].first == v) {
                            used[u][j] = true;
                            break;
                        }
                    }
                }

                stack.append(u);
                foundEdge = true;
                break;
            }
        }

        if (!foundEdge) {
            circuit.append(stack.back());
            stack.removeLast();
        }
    }

    // 反转得到正确顺序
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

// ──────────────────────────────────────────────
// 私有方法 — 奇度顶点匹配
// ──────────────────────────────────────────────

/**
 * @brief 对奇度顶点进行最短路径配对
 *
 * 使用 Floyd-Warshall 计算所有顶点对之间的最短路径，
 * 然后用贪心策略配对奇度顶点，沿最短路径添加重复边。
 */
void ChinesePostman2::matchOddVertices()
{
    // 计算奇度顶点列表
    QVector<int> oddVertices;
    for (int v = 0; v < m_n; ++v) {
        if (m_adj[v].size() % 2 != 0) {
            oddVertices.append(v);
        }
    }

    if (oddVertices.size() % 2 != 0) return; // 不可能的情况

    // Floyd-Warshall 最短路径
    const double INF = 1e18;
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, INF));
    QVector<QVector<int>> next(m_n, QVector<int>(m_n, -1));

    for (int u = 0; u < m_n; ++u) {
        dist[u][u] = 0.0;
        next[u][u] = u;
        for (const auto& edge : m_adj[u]) {
            if (edge.second < dist[u][edge.first]) {
                dist[u][edge.first] = edge.second;
                next[u][edge.first] = edge.first;
            }
        }
    }

    for (int k = 0; k < m_n; ++k) {
        for (int i = 0; i < m_n; ++i) {
            for (int j = 0; j < m_n; ++j) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }

    // 贪心配对：每次选最近的两个奇度顶点配对
    QVector<bool> paired(oddVertices.size(), false);
    for (int i = 0; i < oddVertices.size(); ++i) {
        if (paired[i]) continue;
        double bestDist = INF;
        int bestJ = -1;
        for (int j = i + 1; j < oddVertices.size(); ++j) {
            if (paired[j]) continue;
            if (dist[oddVertices[i]][oddVertices[j]] < bestDist) {
                bestDist = dist[oddVertices[i]][oddVertices[j]];
                bestJ = j;
            }
        }
        if (bestJ >= 0) {
            paired[i] = true;
            paired[bestJ] = true;

            // 沿最短路径添加重复边
            int u = oddVertices[i];
            int v = oddVertices[bestJ];
            while (u != v) {
                int nxt = next[u][v];
                // 添加边 u->nxt
                double w = 0.0;
                for (const auto& edge : m_adj[u]) {
                    if (edge.first == nxt) {
                        w = edge.second;
                        break;
                    }
                }
                m_adj[u].append({nxt, w});
                if (!m_directed) {
                    m_adj[nxt].append({u, w});
                }
                u = nxt;
            }
        }
    }
}
