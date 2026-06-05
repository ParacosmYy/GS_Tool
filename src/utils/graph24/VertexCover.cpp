/**
 * @file VertexCover.cpp
 * @brief 最小顶点覆盖近似算法实现 — 2-近似保证
 */

#include "utils/graph24/VertexCover.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
VertexCover::VertexCover(QObject* parent)
    : QObject(parent)
    , m_vertexCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 从边列表建图 @param edges 边列表 @param vertexCount 顶点数量 */
void VertexCover::buildFromEdges(const QList<Edge>& edges, int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    clear();

    /* 自动推断顶点数 */
    if (vertexCount <= 0) {
        int maxV = 0;
        for (const auto& e : edges) {
            maxV = qMax(maxV, qMax(e.u, e.v));
        }
        m_vertexCount = maxV + 1;
    } else {
        m_vertexCount = vertexCount;
    }

    /* 构建邻接表 */
    m_adjList.resize(m_vertexCount);
    for (const auto& e : edges) {
        addEdge(e.u, e.v);
    }

    m_stats.totalGraphsBuilt++;

    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalGraphsBuilt));
}

/** @brief 添加一条边 @param u 顶点u @param v 顶点v */
void VertexCover::addEdge(int u, int v)
{
    if (u == v) return; /* 忽略自环 */

    /* 确保邻接表足够大 */
    int maxV = qMax(u, v) + 1;
    if (maxV > m_vertexCount) {
        m_vertexCount = maxV;
        m_adjList.resize(m_vertexCount);
    }

    /* 避免重复边 */
    if (!m_adjList[u].contains(v)) {
        m_adjList[u].append(v);
        m_adjList[v].append(u);
        m_edges.append({u, v});
    }
}

/** @brief 计算2-近似最小顶点覆盖 @return 被选中的顶点集合 */
QSet<int> VertexCover::computeApproximateCover()
{
    QElapsedTimer timer;
    timer.start();

    QSet<int> cover;
    QSet<QPair<int, int>> coveredEdges;

    /* 标记已覆盖的边 */
    auto edgeKey = [](int u, int v) -> QPair<int, int> {
        return qMakePair(qMin(u, v), qMax(u, v));
    };

    /* 经典2-近似贪心: 逐边扫描，取两端顶点 */
    for (const auto& e : m_edges) {
        auto key = edgeKey(e.u, e.v);
        if (coveredEdges.contains(key)) continue;

        /* 将u和v都加入覆盖 */
        cover.insert(e.u);
        cover.insert(e.v);
        coveredEdges.insert(key);

        /* 标记u和v的所有邻接边为已覆盖 */
        for (int nbr : m_adjList[e.u]) {
            coveredEdges.insert(edgeKey(e.u, nbr));
        }
        for (int nbr : m_adjList[e.v]) {
            coveredEdges.insert(edgeKey(e.v, nbr));
        }
    }

    /* 更新统计 */
    m_stats.totalCoversComputed++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalCoversComputed);

    emit coverComputed(cover.size(), m_edges.size());
    return cover;
}

/** @brief 带权重的贪心顶点覆盖 @param weights 顶点权重 @return 覆盖集 */
QSet<int> VertexCover::computeWeightedCover(const QVector<double>& weights)
{
    QElapsedTimer timer;
    timer.start();

    QSet<int> cover;
    /* 跟踪每个顶点的剩余度数 */
    QVector<int> remainingDeg(m_vertexCount, 0);
    for (int i = 0; i < m_vertexCount; ++i) {
        remainingDeg[i] = m_adjList[i].size();
    }

    /* 复制边集用于标记 */
    QSet<QPair<int, int>> uncovered;
    for (const auto& e : m_edges) {
        int a = qMin(e.u, e.v), b = qMax(e.u, e.v);
        uncovered.insert(qMakePair(a, b));
    }

    /* 贪心选择: 度数/权重比最大的顶点 */
    while (!uncovered.isEmpty()) {
        int bestV = -1;
        double bestRatio = -1.0;

        for (int v = 0; v < m_vertexCount; ++v) {
            if (remainingDeg[v] <= 0) continue;
            double w = (v < weights.size()) ? qMax(weights[v], 1e-10) : 1.0;
            double ratio = static_cast<double>(remainingDeg[v]) / w;
            if (ratio > bestRatio) {
                bestRatio = ratio;
                bestV = v;
            }
        }

        if (bestV < 0) break;
        cover.insert(bestV);

        /* 移除bestV的所有未覆盖边 */
        for (int nbr : m_adjList[bestV]) {
            int a = qMin(bestV, nbr), b = qMax(bestV, nbr);
            auto key = qMakePair(a, b);
            if (uncovered.contains(key)) {
                uncovered.remove(key);
                remainingDeg[bestV]--;
                remainingDeg[nbr]--;
            }
        }
    }

    m_stats.totalCoversComputed++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalCoversComputed);

    emit coverComputed(cover.size(), m_edges.size());
    return cover;
}

/** @brief 验证覆盖集 @param cover 候选覆盖集 @return 是否合法覆盖 */
bool VertexCover::verifyCover(const QSet<int>& cover) const
{
    for (const auto& e : m_edges) {
        if (!cover.contains(e.u) && !cover.contains(e.v)) {
            return false; /* 这条边未被覆盖 */
        }
    }
    return true;
}

/** @brief 获取边列表 @return 边列表 */
QList<VertexCover::Edge> VertexCover::edges() const
{
    return m_edges;
}

/** @brief 获取顶点数量 @return 顶点数 */
int VertexCover::vertexCount() const
{
    return m_vertexCount;
}

/** @brief 获取边数量 @return 边数 */
int VertexCover::edgeCount() const
{
    return m_edges.size();
}

/** @brief 清空图 */
void VertexCover::clear()
{
    m_adjList.clear();
    m_edges.clear();
    m_vertexCount = 0;
}

/** @brief 重置统计信息 */
void VertexCover::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
