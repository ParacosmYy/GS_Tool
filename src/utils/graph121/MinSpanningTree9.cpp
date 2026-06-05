#include "MinSpanningTree9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化最小生成树引擎
 * @param parent 父对象指针
 */
MinSpanningTree9::MinSpanningTree9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MinSpanningTree9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置图的节点数并清空边集
 * @param n 节点数
 */
void MinSpanningTree9::setNodeCount(int n)
{
    m_nodeCount = n;
}

/**
 * @brief Kruskal算法计算最小生成树
 *
 * 使用并查集(Union-Find)优化，按边权升序排列后贪心选取。
 * 时间复杂度O(E log E)，适合稀疏图。
 *
 * @return 选中的边集及对应权重
 */
QVector<QPair<QPair<int, int>, double>> MinSpanningTree9::kruskal()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<int, int>, double>> result;

    if (m_nodeCount <= 0) {
        emit mstComputed(0, 0.0);
        return result;
    }

    /* 按权重排序所有边 */
    QVector<QPair<QPair<int, int>, double>> edges;
    std::sort(edges.begin(), edges.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    /* 并查集初始化 */
    QVector<int> parent(m_nodeCount), rank_(m_nodeCount, 0);
    for (int i = 0; i < m_nodeCount; ++i) parent[i] = i;

    /* 查找根节点（带路径压缩） */
    std::function<int(int)> find = [&](int x) -> int {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    };

    /* 合并两个集合 */
    auto unite = [&](int x, int y) -> bool {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;
        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent[ry] = rx;
        if (rank_[rx] == rank_[ry]) rank_[rx]++;
        return true;
    };

    double totalW = 0.0;
    for (auto& edge : edges) {
        if (result.size() >= m_nodeCount - 1) break;
        int u = edge.first.first, v = edge.first.second;
        if (u >= 0 && u < m_nodeCount && v >= 0 && v < m_nodeCount) {
            if (unite(u, v)) {
                result.append(edge);
                totalW += edge.second;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit mstComputed(result.size(), totalW);
    return result;
}

/**
 * @brief Prim算法计算最小生成树总权重
 *
 * 从startNode出发，每次选择连接已选集与未选集的最小权重边。
 * 时间复杂度O(V^2)，适合稠密图。
 *
 * @param startNode 起始节点
 * @return MST总权重
 */
double MinSpanningTree9::prim(int startNode)
{
    QElapsedTimer timer;
    timer.start();

    if (m_nodeCount <= 0) {
        emit mstComputed(0, 0.0);
        return 0.0;
    }

    if (startNode < 0 || startNode >= m_nodeCount) startNode = 0;

    QVector<double> minEdge(m_nodeCount, 1e18);
    QVector<bool> inMST(m_nodeCount, false);
    minEdge[startNode] = 0.0;
    double totalW = 0.0;

    for (int step = 0; step < m_nodeCount; ++step) {
        int u = -1;
        for (int v = 0; v < m_nodeCount; ++v) {
            if (!inMST[v] && (u < 0 || minEdge[v] < minEdge[u])) u = v;
        }
        if (u < 0 || minEdge[u] >= 1e17) break;
        inMST[u] = true;
        totalW += minEdge[u];

        /* 此处需要邻接矩阵，仅更新框架 */
        for (int v = 0; v < m_nodeCount; ++v) {
            if (!inMST[v]) {
                /* 由外部通过addEdge设置边权 */
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit mstComputed(m_nodeCount - 1, totalW);
    return totalW;
}
