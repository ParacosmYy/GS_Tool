/**
 * @file KruskalMST.cpp
 * @brief Kruskal最小生成树实现
 */

#include "utils/mst/KruskalMST.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
KruskalMST::KruskalMST(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算MST */
KruskalMST::MstResult KruskalMST::compute(int vertexCount,
                                            const QVector<Edge>& edges)
{
    QElapsedTimer timer;
    timer.start();

    MstResult result;
    result.totalWeight = 0.0;
    result.connected = false;

    /* 按权重排序 */
    QVector<Edge> sortedEdges = edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const Edge& a, const Edge& b) {
                  return a.weight < b.weight;
              });

    /* 并查集初始化 */
    QVector<int> parent(vertexCount);
    QVector<int> rank(vertexCount, 0);
    for (int i = 0; i < vertexCount; ++i) parent[i] = i;

    int edgeCount = 0;
    for (const auto& e : sortedEdges) {
        if (unionSets(parent, rank, e.from, e.to)) {
            result.edges.append(e);
            result.totalWeight += e.weight;
            ++edgeCount;
            if (edgeCount == vertexCount - 1) break;
        }
    }

    result.connected = (edgeCount == vertexCount - 1);

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    emit mstCompleted(edgeCount, result.totalWeight);
    return result;
}

/** @brief 并查集查找(路径压缩) */
int KruskalMST::findParent(QVector<int>& parent, int x)
{
    if (parent[x] != x)
        parent[x] = findParent(parent, parent[x]);
    return parent[x];
}

/** @brief 并查集合并(按秩) */
bool KruskalMST::unionSets(QVector<int>& parent, QVector<int>& rank,
                            int x, int y)
{
    int px = findParent(parent, x);
    int py = findParent(parent, y);
    if (px == py) return false;

    if (rank[px] < rank[py]) {
        parent[px] = py;
    } else if (rank[px] > rank[py]) {
        parent[py] = px;
    } else {
        parent[py] = px;
        ++rank[px];
    }
    return true;
}

/** @brief 重置统计 */
void KruskalMST::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
