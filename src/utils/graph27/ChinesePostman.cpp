/**
 * @file ChinesePostman.cpp
 * @brief 中国邮路算法实现 — Floyd-Warshall + 最小完美匹配 + Hierholzer欧拉回路
 */

#include "utils/graph27/ChinesePostman.h"

#include <QElapsedTimer>
#include <QStack>
#include <QtMath>

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>

/* ── 构造/重置 ── */

/** @brief 构造函数 @param parent 父对象 */
ChinesePostman::ChinesePostman(QObject* parent)
    : QObject(parent)
    , m_nodeCount(0)
    , m_dirty(true)
{
}

/** @brief 设置顶点数 @param n 顶点数 */
void ChinesePostman::setNodeCount(int n)
{
    m_nodeCount = qMax(0, n);
    m_dirty = true;
}

/** @brief 添加边 @param edge 边 */
void ChinesePostman::addEdge(const Edge& edge)
{
    m_edges.append(edge);
    m_dirty = true;
}

/** @brief 清空图 */
void ChinesePostman::clear()
{
    m_nodeCount = 0;
    m_edges.clear();
    m_dist.clear();
    m_next.clear();
    m_dirty = true;
}

/** @brief 检查是否为欧拉图(所有顶点度数为偶数) */
bool ChinesePostman::isEulerian() const
{
    if (m_nodeCount <= 0) return false;

    /* 计算每个顶点的度数 */
    QVector<int> degree(m_nodeCount, 0);
    for (const auto& e : m_edges) {
        ++degree[e.from];
        if (!e.directed) {
            ++degree[e.to];
        }
    }

    for (int d : degree) {
        if (d % 2 != 0) return false;
    }
    return true;
}

/* ── 求解入口 ── */

/** @brief 求解中国邮路问题 @return 遍历结果 */
ChinesePostman::PostmanResult ChinesePostman::solve()
{
    PostmanResult result;
    if (m_nodeCount <= 0 || m_edges.isEmpty()) return result;

    QElapsedTimer timer;
    timer.start();

    /* 若需要重建最短路矩阵 */
    if (m_dirty) {
        floydWarshall();
        m_dirty = false;
    }

    /* 检查图连通性 */
    for (int i = 0; i < m_nodeCount; ++i) {
        if (m_dist[0][i] >= 1e18) return result;
    }

    result.isEulerian = isEulerian();
    if (result.isEulerian) {
        /* 已是欧拉图，直接找欧拉回路 */
        QVector<QList<QPair<int,double>>> adj(m_nodeCount);
        for (const auto& e : m_edges) {
            adj[e.from].append({e.to, e.weight});
            if (!e.directed) {
                adj[e.to].append({e.from, e.weight});
            }
        }
        result.route = findEulerCircuit(adj, 0);
        result.augmentedEdges = 0;
    } else {
        /* 需要增广: 找奇度顶点并做最小权重完美匹配 */
        QVector<int> oddNodes = findOddDegreeNodes();
        if (oddNodes.size() % 2 != 0) return result;

        auto matching = minimumWeightMatching(oddNodes);
        auto adj = buildAugmentedGraph(matching);

        result.augmentedEdges = static_cast<int>(matching.size());
        result.route = findEulerCircuit(adj, 0);
    }

    /* 计算总代价 */
    result.totalCost = 0.0;
    for (int i = 0; i < result.route.size() - 1; ++i) {
        int u = result.route[i];
        int v = result.route[i + 1];
        result.totalCost += m_dist[u][v];
    }
    result.valid = !result.route.isEmpty();

    /* 更新统计 */
    ++m_stats.totalSolves;
    m_stats.totalNodesProcessed += m_nodeCount;
    m_stats.totalEdgesProcessed += static_cast<int>(m_edges.size());
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(result.totalCost, result.route.size());
    return result;
}

/* ── 私有: Floyd-Warshall ── */

/** @brief Floyd-Warshall全源最短路 */
void ChinesePostman::floydWarshall()
{
    const double INF = 1e18;
    m_dist.assign(m_nodeCount, QVector<double>(m_nodeCount, INF));
    m_next.assign(m_nodeCount, QVector<int>(m_nodeCount, -1));

    for (int i = 0; i < m_nodeCount; ++i) {
        m_dist[i][i] = 0.0;
        m_next[i][i] = i;
    }

    for (const auto& e : m_edges) {
        if (e.weight < m_dist[e.from][e.to]) {
            m_dist[e.from][e.to] = e.weight;
            m_next[e.from][e.to] = e.to;
        }
        if (!e.directed && e.weight < m_dist[e.to][e.from]) {
            m_dist[e.to][e.from] = e.weight;
            m_next[e.to][e.from] = e.from;
        }
    }

    for (int k = 0; k < m_nodeCount; ++k) {
        for (int i = 0; i < m_nodeCount; ++i) {
            for (int j = 0; j < m_nodeCount; ++j) {
                if (m_dist[i][k] + m_dist[k][j] < m_dist[i][j]) {
                    m_dist[i][j] = m_dist[i][k] + m_dist[k][j];
                    m_next[i][j] = m_next[i][k];
                }
            }
        }
    }
}

/* ── 私有: 查找奇度顶点 ── */

/** @brief 查找所有奇度顶点 */
QVector<int> ChinesePostman::findOddDegreeNodes() const
{
    QVector<int> degree(m_nodeCount, 0);
    for (const auto& e : m_edges) {
        ++degree[e.from];
        if (!e.directed) ++degree[e.to];
    }

    QVector<int> oddNodes;
    for (int i = 0; i < m_nodeCount; ++i) {
        if (degree[i] % 2 != 0) oddNodes.append(i);
    }
    return oddNodes;
}

/* ── 私有: 最小权重完美匹配 ── */

/** @brief 暴力搜索最小权重完美匹配(适用于小规模奇度顶点) */
QVector<QPair<int,int>> ChinesePostman::minimumWeightMatching(
    const QVector<int>& oddNodes)
{
    QVector<QPair<int,int>> bestMatching;
    double bestCost = 1e18;
    int n = oddNodes.size();

    /* 递归配对 */
    QVector<bool> used(n, false);
    QVector<QPair<int,int>> current;

    std::function<void(double)> search = [&](double cost) {
        if (cost >= bestCost) return;

        /* 找第一个未配对的 */
        int first = -1;
        for (int i = 0; i < n; ++i) {
            if (!used[i]) { first = i; break; }
        }
        if (first == -1) {
            /* 全部配对完成 */
            if (cost < bestCost) {
                bestCost = cost;
                bestMatching = current;
            }
            return;
        }

        used[first] = true;
        for (int j = first + 1; j < n; ++j) {
            if (used[j]) continue;
            used[j] = true;
            current.append({oddNodes[first], oddNodes[j]});
            search(cost + m_dist[oddNodes[first]][oddNodes[j]]);
            current.removeLast();
            used[j] = false;
        }
        used[first] = false;
    };

    search(0.0);
    return bestMatching;
}

/* ── 私有: 构造增广图 ── */

/** @brief 构造欧拉增广邻接表(原始边 + 匹配路径上的额外边) */
QVector<QList<QPair<int,double>>> ChinesePostman::buildAugmentedGraph(
    const QVector<QPair<int,int>>& matching)
{
    QVector<QList<QPair<int,double>>> adj(m_nodeCount);

    /* 添加原始边 */
    for (const auto& e : m_edges) {
        adj[e.from].append({e.to, e.weight});
        if (!e.directed) {
            adj[e.to].append({e.from, e.weight});
        }
    }

    /* 沿最短路添加匹配路径上的额外边 */
    for (const auto& m : matching) {
        int u = m.first, v = m.second;
        /* 用 m_next 重建路径并添加每条边 */
        int cur = u;
        while (cur != v) {
            int nxt = m_next[cur][v];
            double w = m_dist[cur][nxt];
            adj[cur].append({nxt, w});
            adj[nxt].append({cur, w});
            cur = nxt;
        }
    }
    return adj;
}

/* ── 私有: Hierholzer欧拉回路 ── */

/** @brief Hierholzer算法在增广邻接表上找欧拉回路 */
QList<int> ChinesePostman::findEulerCircuit(
    QVector<QList<QPair<int,double>>>& adj, int start)
{
    QList<int> circuit;
    if (adj.isEmpty()) return circuit;

    QStack<int> stack;
    stack.push(start);

    while (!stack.isEmpty()) {
        int v = stack.top();
        if (adj[v].isEmpty()) {
            circuit.prepend(v);
            stack.pop();
        } else {
            auto edge = adj[v].takeFirst();
            stack.push(edge.first);
        }
    }

    return circuit;
}

/* ── 统计 ── */

/** @brief 重置统计 */
void ChinesePostman::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
