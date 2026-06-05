/**
 * @file MinSpanningTree8.cpp
 * @brief 最小生成树求解器实现
 *
 * 支持 Kruskal 算法(基于并查集)和 Prim 算法(基于优先队列)，
 * 可处理连通图(最小生成树)和非连通图(最小生成森林)。
 */

#include "utils/graph91/MinSpanningTree8.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

/* ──────────────────── 构造/重置 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
MinSpanningTree8::MinSpanningTree8(QObject* parent)
    : QObject(parent)
{
}

/** @brief 重置统计数据 */
void MinSpanningTree8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ──────────────────── 配置 ──────────────────── */

/**
 * @brief 设置求解算法
 * @param algorithm 算法名: "kruskal" 或 "prim"
 */
void MinSpanningTree8::setAlgorithm(const QString& algorithm)
{
    if (algorithm == "kruskal" || algorithm == "prim") {
        m_algorithm = algorithm;
    }
}

/* ──────────────────── 主求解入口 ──────────────────── */

/**
 * @brief 求解最小生成树(或森林)
 * @param vertexCount 顶点数
 * @param edges 边列表，每条边为 ((u, v), weight)
 * @return 生成树中的边列表 [(u, v), ...]
 *
 * 根据配置选择 Kruskal 或 Prim 算法。
 * 非连通图返回最小生成森林。
 */
QVector<QPair<int, int>> MinSpanningTree8::solve(
    int vertexCount,
    const QVector<QPair<QPair<int, int>, double>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    m_totalWeight = 0.0;
    m_connected = false;

    if (vertexCount <= 0 || edges.isEmpty()) {
        emit treeComputed(0, 0.0);
        return {};
    }

    QVector<QPair<int, int>> result;

    if (m_algorithm == "kruskal") {
        result = kruskalSolve(vertexCount, edges);
    } else {
        result = primSolve(vertexCount, edges);
    }

    /* 判断图是否连通: 生成树边数 = 顶点数 - 1 */
    m_connected = (static_cast<int>(result.size()) == vertexCount - 1);

    /* 更新统计 */
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalTreesComputed;
    m_stats.totalEdges += static_cast<int>(result.size());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTreesComputed);

    emit treeComputed(static_cast<int>(result.size()), m_totalWeight);
    return result;
}

/**
 * @brief 获取生成树总权重
 * @return 最近一次求解的总权重
 */
double MinSpanningTree8::totalWeight() const
{
    return m_totalWeight;
}

/**
 * @brief 检查图是否连通
 * @return true表示图连通(生成树边数=顶点数-1)
 */
bool MinSpanningTree8::isConnected() const
{
    return m_connected;
}

/* ──────────────────── Kruskal 算法 ──────────────────── */

/**
 * @brief Kruskal算法求解MST
 * @param n 顶点数
 * @param edges 边列表
 * @return 生成树边列表
 *
 * 将边按权重排序，依次选取不成环的边加入生成树。
 * 使用并查集(路径压缩+按秩合并)检测环。
 */
QVector<QPair<int, int>> MinSpanningTree8::kruskalSolve(
    int n,
    const QVector<QPair<QPair<int, int>, double>>& edges)
{
    /* 将边按权重排序 */
    QVector<int> sortedIdx(edges.size());
    std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
    std::sort(sortedIdx.begin(), sortedIdx.end(), [&edges](int a, int b) {
        return edges[a].second < edges[b].second;
    });

    /* 并查集初始化 */
    QVector<int> parent(n);
    QVector<int> rank(n, 0);
    std::iota(parent.begin(), parent.end(), 0);

    /* 并查集: 路径压缩查找 */
    std::function<int(int)> findSet = [&](int x) -> int {
        if (parent[x] != x) {
            parent[x] = findSet(parent[x]);
        }
        return parent[x];
    };

    /* 并查集: 按秩合并 */
    auto unionSet = [&](int x, int y) -> bool {
        int rx = findSet(x);
        int ry = findSet(y);
        if (rx == ry) return false;
        if (rank[rx] < rank[ry]) {
            parent[rx] = ry;
        } else if (rank[rx] > rank[ry]) {
            parent[ry] = rx;
        } else {
            parent[ry] = rx;
            ++rank[rx];
        }
        return true;
    };

    QVector<QPair<int, int>> mst;
    m_totalWeight = 0.0;

    for (int idx : sortedIdx) {
        int u = edges[idx].first.first;
        int v = edges[idx].first.second;
        double w = edges[idx].second;

        /* 边界检查 */
        if (u < 0 || u >= n || v < 0 || v >= n) continue;

        if (unionSet(u, v)) {
            mst.append({u, v});
            m_totalWeight += w;

            /* 生成树已满 n-1 条边 */
            if (static_cast<int>(mst.size()) == n - 1) break;
        }
    }

    return mst;
}

/* ──────────────────── Prim 算法 ──────────────────── */

/**
 * @brief Prim算法求解MST
 * @param n 顶点数
 * @param edges 边列表
 * @return 生成树边列表
 *
 * 从顶点0出发，使用优先队列每次选取连接已访问集合与
 * 未访问集合的最小权重边。
 */
QVector<QPair<int, int>> MinSpanningTree8::primSolve(
    int n,
    const QVector<QPair<QPair<int, int>, double>>& edges)
{
    /* 构建邻接表 */
    QVector<QVector<QPair<int, double>>> adj(n);
    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double w = edge.second;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u].append({v, w});
        adj[v].append({u, w});
    }

    QVector<bool> inMST(n, false);
    QVector<double> minEdge(n, std::numeric_limits<double>::infinity());
    QVector<int> parent(n, -1);

    /* 优先队列: (权重, 顶点) */
    using PEdge = QPair<double, int>;
    std::priority_queue<PEdge, std::vector<PEdge>, std::greater<PEdge>> pq;

    /* 从顶点0开始 */
    minEdge[0] = 0.0;
    pq.push({0.0, 0});

    QVector<QPair<int, int>> mst;
    m_totalWeight = 0.0;

    while (!pq.empty()) {
        auto [weight, u] = pq.top();
        pq.pop();

        if (inMST[u]) continue;
        inMST[u] = true;

        /* 将该边加入生成树(跳过起始顶点) */
        if (parent[u] >= 0) {
            mst.append({parent[u], u});
            m_totalWeight += weight;
        }

        /* 松弛邻居 */
        for (const auto& [to, w] : adj[u]) {
            if (!inMST[to] && w < minEdge[to]) {
                minEdge[to] = w;
                parent[to] = u;
                pq.push({w, to});
            }
        }
    }

    return mst;
}
