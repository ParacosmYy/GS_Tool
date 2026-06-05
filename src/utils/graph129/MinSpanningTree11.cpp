#include "MinSpanningTree11.h"
#include <QElapsedTimer>
#include <QMap>
#include <QSet>
#include <algorithm>

/**
 * @brief 构造函数，初始化最小生成树引擎v11
 * @param parent 父对象指针
 */
MinSpanningTree11::MinSpanningTree11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MinSpanningTree11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Kruskal算法求最小生成树
 *
 * 按边权升序排列后贪心选取，使用并查集维护连通性。
 * 时间复杂度O(E log E)。
 *
 * @param edges 边列表 (起点, 终点, 权重)
 * @param vertexCount 顶点数
 * @return MST边列表
 */
QVector<QPair<QPair<int, int>, double>> MinSpanningTree11::kruskal(
    const QVector<QPair<QPair<int, int>, double>>& edges, int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<int, int>, double>> mst;

    if (vertexCount <= 0) {
        emit solveCompleted(0);
        return mst;
    }

    /* 按权重排序 */
    auto sortedEdges = edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    /* 并查集初始化 */
    QVector<int> parent(vertexCount), rank_(vertexCount, 0);
    for (int i = 0; i < vertexCount; ++i) parent[i] = i;

    std::function<int(int)> find = [&](int x) -> int {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    };

    auto unite = [&](int x, int y) -> bool {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;
        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent[ry] = rx;
        if (rank_[rx] == rank_[ry]) rank_[rx]++;
        return true;
    };

    for (const auto& edge : sortedEdges) {
        if (mst.size() >= vertexCount - 1) break;
        int u = edge.first.first, v = edge.first.second;
        if (u >= 0 && u < vertexCount && v >= 0 && v < vertexCount) {
            if (unite(u, v)) mst.append(edge);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(mst.size());
    return mst;
}

/**
 * @brief Prim算法求最小生成树
 *
 * 使用优先队列优化，从顶点0出发扩展MST。
 * 时间复杂度O(V^2)（邻接矩阵版本）。
 *
 * @param adjacencyMatrix 邻接矩阵（权重为0表示不连通）
 * @return MST边列表
 */
QVector<QPair<QPair<int, int>, double>> MinSpanningTree11::prim(
    const QVector<QVector<double>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<QPair<QPair<int, int>, double>> mst;

    if (n == 0) {
        emit solveCompleted(0);
        return mst;
    }

    QVector<double> minCost(n, 1e18);
    QVector<int> parent(n, -1);
    QVector<bool> inMST(n, false);

    minCost[0] = 0.0;

    for (int step = 0; step < n; ++step) {
        /* 选择当前最小代价的未加入顶点 */
        int u = -1;
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && (u < 0 || minCost[v] < minCost[u])) u = v;
        }
        if (u < 0 || minCost[u] >= 1e17) break;

        inMST[u] = true;
        if (parent[u] >= 0) {
            mst.append({{parent[u], u}, minCost[u]});
        }

        /* 更新邻居的最小代价 */
        for (int v = 0; v < qMin(n, adjacencyMatrix[u].size()); ++v) {
            double w = adjacencyMatrix[u][v];
            if (!inMST[v] && w > 0 && w < minCost[v]) {
                minCost[v] = w;
                parent[v] = u;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(mst.size());
    return mst;
}

/**
 * @brief 计算最小生成森林（非连通图）
 *
 * 对每个连通分量分别运行Kruskal算法。
 *
 * @param edges 边列表
 * @param vertexCount 顶点数
 * @return 各连通分量的MST边列表
 */
QVector<QVector<QPair<QPair<int, int>, double>>> MinSpanningTree11::minSpanningForest(
    const QVector<QPair<QPair<int, int>, double>>& edges, int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QPair<QPair<int, int>, double>>> forest;

    if (vertexCount <= 0) {
        emit solveCompleted(0);
        return forest;
    }

    /* 先用Kruskal获取所有MST边 */
    auto mstEdges = kruskal(edges, vertexCount);

    /* 并查集分组 */
    QVector<int> parent(vertexCount);
    for (int i = 0; i < vertexCount; ++i) parent[i] = i;

    std::function<int(int)> find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    for (const auto& e : mstEdges) {
        int u = e.first.first, v = e.first.second;
        int ru = find(u), rv = find(v);
        if (ru != rv) parent[rv] = ru;
    }

    /* 按连通分量分组 */
    QMap<int, QVector<QPair<QPair<int, int>, double>>> groups;
    for (const auto& e : mstEdges) {
        int root = find(e.first.first);
        groups[root].append(e);
    }

    /* 孤立节点也作为单独的森林 */
    QSet<int> seenRoots;
    for (const auto& e : mstEdges) {
        int root = find(e.first.first);
        if (!seenRoots.contains(root)) {
            seenRoots.insert(root);
        }
    }

    for (auto it = groups.begin(); it != groups.end(); ++it) {
        forest.append(it.value());
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    emit solveCompleted(mstEdges.size());
    return forest;
}

/**
 * @brief 计算MST总权重
 * @param mstEdges MST边列表
 * @return 总权重
 */
double MinSpanningTree11::totalWeight(
    const QVector<QPair<QPair<int, int>, double>>& mstEdges) const
{
    double sum = 0.0;
    for (const auto& e : mstEdges) {
        sum += e.second;
    }
    return sum;
}
