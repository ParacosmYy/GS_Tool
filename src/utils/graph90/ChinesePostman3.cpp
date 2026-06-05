/**
 * @file ChinesePostman3.cpp
 * @brief 中国邮递员问题求解器实现
 *
 * 求解经过图中所有边至少一次的最短闭合路径。
 * 算法流程:
 *   1. 检查图是否为欧拉图(所有顶点度数为偶数)
 *   2. 若非欧拉图，找出所有奇度顶点
 *   3. 对奇度顶点求最短路径全对距离
 *   4. 使用最小权完美匹配配对奇度顶点
 *   5. 在匹配顶点间复制最短路径上的边
 *   6. 在增广图上求欧拉回路(Fleury/Hierholzer)
 */

#include "utils/graph90/ChinesePostman3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

/* ──────────────────── 构造/重置 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
ChinesePostman3::ChinesePostman3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 重置统计数据 */
void ChinesePostman3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ──────────────────── 图构建 ──────────────────── */

/**
 * @brief 设置带权无向图
 * @param adjList 邻接表，adjList[u] = [(v, weight), ...]
 */
void ChinesePostman3::setGraph(const QVector<QVector<QPair<int, double>>>& adjList)
{
    m_adjList = adjList;
    m_tourWeight = 0.0;
}

/* ──────────────────── 主求解入口 ──────────────────── */

/**
 * @brief 求解最优邮递员路径
 * @return 顶点访问序列(闭合路径)
 *
 * 步骤: 找奇度顶点 -> 全对最短路 -> 最小权完美匹配 -> 增广图 -> 欧拉回路
 */
QVector<int> ChinesePostman3::solve()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_adjList.size();
    if (n == 0) {
        emit tourComputed(0, 0.0);
        return {};
    }

    /* 统计边数 */
    int edgeCount = 0;
    for (int i = 0; i < n; ++i) {
        edgeCount += m_adjList[i].size();
    }
    edgeCount /= 2; /* 无向图每条边出现两次 */

    /* 计算每个顶点的度数 */
    QVector<int> degree(n, 0);
    for (int u = 0; u < n; ++u) {
        degree[u] = m_adjList[u].size();
    }

    /* 找出奇度顶点 */
    QVector<int> oddVertices;
    for (int v = 0; v < n; ++v) {
        if (degree[v] % 2 != 0) {
            oddVertices.append(v);
        }
    }

    /* 构建增广图的邻接表(可修改) */
    QVector<QVector<QPair<int, double>>> augmented = m_adjList;

    /* 若存在奇度顶点，执行最小权完美匹配 */
    if (!oddVertices.isEmpty()) {
        int m = oddVertices.size();

        /* Floyd-Warshall 求全对最短路 */
        QVector<QVector<double>> dist(n, QVector<double>(n,
            std::numeric_limits<double>::infinity()));
        QVector<QVector<int>> nextHop(n, QVector<int>(n, -1));

        for (int u = 0; u < n; ++u) {
            dist[u][u] = 0.0;
            nextHop[u][u] = u;
            for (const auto& edge : m_adjList[u]) {
                int v = edge.first;
                double w = edge.second;
                if (w < dist[u][v]) {
                    dist[u][v] = w;
                    nextHop[u][v] = v;
                }
            }
        }

        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        nextHop[i][j] = nextHop[i][k];
                    }
                }
            }
        }

        /* 构建奇度顶点之间的代价矩阵 */
        QVector<QVector<double>> costMatrix(m, QVector<double>(m, 0.0));
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                costMatrix[i][j] = dist[oddVertices[i]][oddVertices[j]];
            }
        }

        /* 求最小权完美匹配(简化匈牙利法/贪心) */
        QVector<QPair<int, int>> matching = minWeightPerfectMatch(costMatrix, m);

        /* 在增广图中沿最短路径添加额外边 */
        for (const auto& match : matching) {
            int u = oddVertices[match.first];
            int v = oddVertices[match.second];

            /* 沿最短路径回溯，逐段添加边 */
            int cur = u;
            while (cur != v && nextHop[cur][v] >= 0) {
                int nxt = nextHop[cur][v];
                double w = dist[cur][nxt];
                augmented[cur].append({nxt, w});
                augmented[nxt].append({cur, w});
                cur = nxt;
            }
        }
    }

    /* 在增广图上求欧拉回路(Hierholzer算法) */
    QVector<int> tour = hierholzerEulerTour(augmented, n);

    /* 计算路径总权重 */
    m_tourWeight = 0.0;
    for (int i = 0; i < static_cast<int>(tour.size()) - 1; ++i) {
        int u = tour[i];
        int v = tour[i + 1];
        /* 在原始图中查找边权重 */
        double w = 0.0;
        for (const auto& edge : m_adjList[u]) {
            if (edge.first == v) {
                w = edge.second;
                break;
            }
        }
        /* 如果原始图中没有，使用增广图的权重 */
        if (w == 0.0) {
            for (const auto& edge : augmented[u]) {
                if (edge.first == v) {
                    w = qMin(w > 0 ? w : std::numeric_limits<double>::max(), edge.second);
                    break;
                }
            }
        }
        m_tourWeight += w;
    }

    /* 更新统计 */
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalToursComputed;
    m_stats.totalEdges = edgeCount;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalToursComputed);

    emit tourComputed(edgeCount, m_tourWeight);
    return tour;
}

/**
 * @brief 获取路径总权重
 * @return 最近一次求解的路径总权重
 */
double ChinesePostman3::tourWeight() const
{
    return m_tourWeight;
}

/**
 * @brief 检查图是否为欧拉图
 * @return true表示所有顶点度数为偶数
 */
bool ChinesePostman3::isEulerian() const
{
    for (int u = 0; u < m_adjList.size(); ++u) {
        if (m_adjList[u].size() % 2 != 0) {
            return false;
        }
    }
    return !m_adjList.isEmpty();
}

/* ──────────────────── 私有方法 ──────────────────── */

/**
 * @brief 最小权完美匹配(贪心近似 + 局部优化)
 * @param cost 代价矩阵
 * @param m 顶点数(必须为偶数)
 * @return 匹配对列表
 *
 * 使用贪心策略: 每次选取代价最小的未匹配对。
 * 对于偶数个顶点产生 m/2 个配对。
 */
QVector<QPair<int, int>> ChinesePostman3::minWeightPerfectMatch(
    const QVector<QVector<double>>& cost, int m)
{
    QVector<QPair<int, int>> matches;
    if (m == 0 || m % 2 != 0) return matches;

    QVector<bool> matched(m, false);

    /* 贪心: 每次找全局最小代价的未匹配对 */
    for (int round = 0; round < m / 2; ++round) {
        double bestCost = std::numeric_limits<double>::infinity();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < m; ++i) {
            if (matched[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (matched[j]) continue;
                if (cost[i][j] < bestCost) {
                    bestCost = cost[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI >= 0 && bestJ >= 0) {
            matched[bestI] = true;
            matched[bestJ] = true;
            matches.append({bestI, bestJ});
        }
    }

    return matches;
}

/**
 * @brief Hierholzer算法求欧拉回路
 * @param adj 邻接表(增广后所有顶点度数为偶数)
 * @param n 顶点数
 * @return 欧拉回路顶点序列
 *
 * 使用栈模拟递归: 从顶点0出发，每次沿未使用的边前进，
 * 无路可走时回退并将顶点加入路径。
 */
QVector<int> ChinesePostman3::hierholzerEulerTour(
    QVector<QVector<QPair<int, double>>>& adj, int n)
{
    if (n == 0) return {};

    /* 计算每个顶点的度数，检查是否为欧拉图 */
    int start = -1;
    for (int i = 0; i < n; ++i) {
        if (!adj[i].isEmpty()) {
            start = i;
            break;
        }
    }
    if (start < 0) return {};

    /* 使用边索引标记已访问的边 */
    QVector<int> edgeIndex(n, 0);

    QVector<int> stack;
    QVector<int> circuit;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();

        if (edgeIndex[v] < static_cast<int>(adj[v].size())) {
            /* 还有未访问的边 */
            int to = adj[v][edgeIndex[v]].first;
            ++edgeIndex[v];

            /* 删除反向边(无向图中每条边有两个方向) */
            bool removed = false;
            for (int i = 0; i < static_cast<int>(adj[to].size()); ++i) {
                if (adj[to][i].first == v && !removed) {
                    /* 标记为已删除(用特殊值替代删除操作) */
                    adj[to][i].first = -1;
                    removed = true;
                    break;
                }
            }

            if (to >= 0) {
                stack.append(to);
            }
        } else {
            /* 所有边已访问，回退 */
            circuit.append(v);
            stack.removeLast();
        }
    }

    /* 反转得到欧拉回路 */
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}
