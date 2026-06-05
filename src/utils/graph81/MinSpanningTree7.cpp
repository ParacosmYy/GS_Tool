/**
 * @file MinSpanningTree7.cpp
 * @brief 最小生成树算法实现（第7版）
 *
 * 实现Prim算法和Kruskal算法两种经典最小生成树算法。
 * Prim使用优先队列从单点扩展，适合稠密图。
 * Kruskal使用并查集按边权排序，适合稀疏图。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph81/MinSpanningTree7.h"

#include <QElapsedTimer>
#include <queue>
#include <algorithm>

/**
 * @brief 构造函数，初始化最小生成树求解器
 * @param parent 父QObject对象指针
 */
MinSpanningTree7::MinSpanningTree7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 */
void MinSpanningTree7::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_edges.clear();
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加无向加权边
 * @param u 第一个端点
 * @param v 第二个端点
 * @param weight 边的权重
 */
void MinSpanningTree7::addEdge(int u, int v, double weight)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_edges.append({weight, {u, v}});
    m_adj[u].append({v, weight});
    m_adj[v].append({u, weight});
}

/**
 * @brief 使用Prim算法求解最小生成树
 *
 * 从顶点0开始，每次选择连接已选顶点集和未选顶点集的最小权重边。
 * 使用优先队列维护候选边，时间复杂度O(E log V)。
 *
 * @return 最小生成树的边集合
 */
QVector<QPair<int,int>> MinSpanningTree7::solvePrim()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int,int>> mstEdges;
    m_weight = 0.0;
    m_connected = false;

    if (m_n <= 1) {
        m_connected = (m_n == 1);
        emit solveCompleted(0, 0.0);
        return mstEdges;
    }

    QVector<bool> inMST(m_n, false);
    /* 优先队列：(权重, (from, to)) */
    std::priority_queue<std::pair<double, std::pair<int,int>>,
                        std::vector<std::pair<double, std::pair<int,int>>>,
                        std::greater<>> pq;

    /* 从顶点0开始 */
    inMST[0] = true;
    for (const auto& edge : m_adj[0]) {
        pq.push({edge.second, {0, edge.first}});
    }

    while (!pq.empty() && mstEdges.size() < m_n - 1) {
        auto top = pq.top();
        pq.pop();

        double w = top.first;
        int from = top.second.first;
        int to = top.second.second;

        if (inMST[to]) continue;

        /* 加入MST */
        inMST[to] = true;
        mstEdges.append({from, to});
        m_weight += w;

        /* 将新顶点的邻边加入优先队列 */
        for (const auto& edge : m_adj[to]) {
            if (!inMST[edge.first]) {
                pq.push({edge.second, {to, edge.first}});
            }
        }
    }

    m_connected = (mstEdges.size() == m_n - 1);

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalEdges += mstEdges.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(mstEdges.size(), m_weight);
    return mstEdges;
}

/**
 * @brief 使用Kruskal算法求解最小生成树
 *
 * 按边权从小到大排序，依次加入不形成环的边。
 * 使用并查集（Union-Find）检测环，时间复杂度O(E log E)。
 *
 * @return 最小生成树的边集合
 */
QVector<QPair<int,int>> MinSpanningTree7::solveKruskal()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int,int>> mstEdges;
    m_weight = 0.0;
    m_connected = false;

    if (m_n <= 1) {
        m_connected = (m_n == 1);
        emit solveCompleted(0, 0.0);
        return mstEdges;
    }

    /* 按权重排序 */
    auto sorted = m_edges;
    std::sort(sorted.begin(), sorted.end());

    /* 并查集 */
    QVector<int> parent(m_n);
    QVector<int> rankVal(m_n, 0);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    /* 路径压缩查找 */
    std::function<int(int)> find = [&](int x) -> int {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    };

    /* 按秩合并 */
    auto unite = [&](int x, int y) -> bool {
        int px = find(x), py = find(y);
        if (px == py) return false;
        if (rankVal[px] < rankVal[py]) std::swap(px, py);
        parent[py] = px;
        if (rankVal[px] == rankVal[py]) rankVal[px]++;
        return true;
    };

    /* 逐边加入 */
    for (const auto& edge : sorted) {
        int u = edge.second.first;
        int v = edge.second.second;
        double w = edge.first;

        if (unite(u, v)) {
            mstEdges.append({u, v});
            m_weight += w;
            if (static_cast<int>(mstEdges.size()) == m_n - 1) break;
        }
    }

    m_connected = (static_cast<int>(mstEdges.size()) == m_n - 1);

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalEdges += mstEdges.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(mstEdges.size(), m_weight);
    return mstEdges;
}

/**
 * @brief 获取当前统计信息
 * @return 求解统计结构
 */
MinSpanningTree7::Stats MinSpanningTree7::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void MinSpanningTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
