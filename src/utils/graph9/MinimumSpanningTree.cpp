/**
 * @file MinimumSpanningTree.cpp
 * @brief 最小生成树实现 — Prim/Kruskal算法
 */

#include "utils/graph9/MinimumSpanningTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
MinimumSpanningTree::MinimumSpanningTree(QObject* parent)
    : QObject(parent)
    , m_numNodes(0)
    , m_totalWeight(0.0)
    , m_timeSum(0.0)
{
}

/** @brief 设置图 @param numNodes 节点数 @param adj 邻接表 */
void MinimumSpanningTree::setGraph(
    int numNodes,
    const QVector<QVector<QPair<int, double>>>& adj)
{
    m_numNodes = numNodes;
    m_adj = adj;
    /* 确保邻接表大小匹配 */
    if (m_adj.size() < numNodes) {
        m_adj.resize(numNodes);
    }
}

/** @brief Prim算法求MST @return MST边集 */
QVector<QPair<int, int>> MinimumSpanningTree::prim()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> edges;
    m_totalWeight = 0.0;

    if (m_numNodes <= 0) {
        m_timeSum += timer.elapsed();
        return edges;
    }
    if (m_numNodes == 1) {
        m_timeSum += timer.elapsed();
        return edges;
    }

    /* key[v]: 到达v的最小边权重 */
    QVector<double> key(m_numNodes, qInf());
    QVector<int> parent(m_numNodes, -1);
    QVector<bool> inMST(m_numNodes, false);

    /* 从节点0开始 */
    key[0] = 0.0;

    /* 优先队列: (权重, 节点) */
    using PNode = QPair<double, int>;
    std::priority_queue<PNode, std::vector<PNode>, std::greater<PNode>> pq;
    pq.push({0.0, 0});

    while (!pq.empty()) {
        auto [w, u] = pq.top();
        pq.pop();

        if (inMST[u]) continue;
        inMST[u] = true;

        if (parent[u] != -1) {
            edges.append({parent[u], u});
            m_totalWeight += key[u];
        }

        /* 遍历u的邻接边 */
        if (u < m_adj.size()) {
            for (const auto& [v, weight] : m_adj[u]) {
                if (!inMST[v] && weight < key[v]) {
                    key[v] = weight;
                    parent[v] = u;
                    pq.push({weight, v});
                }
            }
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit mstComputed(tr("prim"), edges.size());
    return edges;
}

/** @brief 并查集查找 @param parent 父节点数组 @param x 查找节点 @return 根节点 */
int MinimumSpanningTree::findRoot(QVector<int>& parent, int x) const
{
    if (parent[x] != x) {
        parent[x] = findRoot(parent, parent[x]); /* 路径压缩 */
    }
    return parent[x];
}

/** @brief 并查集合并 @param parent 父节点数组 @param rank 秩数组 @param x 节点x @param y 节点y @return 是否合并成功 */
bool MinimumSpanningTree::unionSet(QVector<int>& parent, QVector<int>& rank,
                                   int x, int y) const
{
    int rx = findRoot(parent, x);
    int ry = findRoot(parent, y);
    if (rx == ry) return false;

    /* 按秩合并 */
    if (rank[rx] < rank[ry]) {
        parent[rx] = ry;
    } else if (rank[rx] > rank[ry]) {
        parent[ry] = rx;
    } else {
        parent[ry] = rx;
        ++rank[rx];
    }
    return true;
}

/** @brief Kruskal算法求MST @return MST边集 */
QVector<QPair<int, int>> MinimumSpanningTree::kruskal()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> mstEdges;
    m_totalWeight = 0.0;

    if (m_numNodes <= 0) {
        m_timeSum += timer.elapsed();
        return mstEdges;
    }

    /* 收集所有边: (权重, u, v) */
    struct Edge {
        double weight;
        int u, v;
        bool operator<(const Edge& o) const { return weight < o.weight; }
    };
    QVector<Edge> allEdges;

    for (int u = 0; u < qMin(m_numNodes, m_adj.size()); ++u) {
        for (const auto& [v, w] : m_adj[u]) {
            if (u < v) { /* 避免重复边 */
                allEdges.append({w, u, v});
            }
        }
    }

    std::sort(allEdges.begin(), allEdges.end());

    /* 并查集初始化 */
    QVector<int> parent(m_numNodes);
    QVector<int> rank(m_numNodes, 0);
    for (int i = 0; i < m_numNodes; ++i) {
        parent[i] = i;
    }

    /* 贪心选边 */
    for (const auto& edge : allEdges) {
        if (unionSet(parent, rank, edge.u, edge.v)) {
            mstEdges.append({edge.u, edge.v});
            m_totalWeight += edge.weight;
            if (static_cast<int>(mstEdges.size()) == m_numNodes - 1) {
                break; /* MST完成 */
            }
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit mstComputed(tr("kruskal"), mstEdges.size());
    return mstEdges;
}

/** @brief 获取MST总权重 @return 总权重 */
double MinimumSpanningTree::totalWeight() const
{
    return m_totalWeight;
}

/** @brief 重置统计 */
void MinimumSpanningTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
