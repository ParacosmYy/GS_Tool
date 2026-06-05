/**
 * @file BipartiteGraph.cpp
 * @brief 二部图匹配实现 — Hopcroft-Karp最大匹配
 *
 * 使用BFS构建层次图 + DFS寻找增广路径的Hopcroft-Karp算法，
 * 时间复杂度 O(E*sqrt(V))，远优于朴素增广的 O(VE)。
 */

#include "utils/graph17/BipartiteGraph.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <algorithm>

// ── 常量 ──

/** @brief 无穷距离标记(BFS层次图中未访问节点) */
static constexpr int kInf = 0x3f3f3f3f;

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
BipartiteGraph::BipartiteGraph(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BipartiteGraph"));
}

// ── 图构建 ──

/**
 * @brief 设置左右分区大小
 * @param leftSize 左侧顶点数
 * @param rightSize 右侧顶点数
 */
void BipartiteGraph::setPartitions(int leftSize, int rightSize)
{
    m_leftSize = qMax(0, leftSize);
    m_rightSize = qMax(0, rightSize);
    m_adj.clear();
    m_matchLeft.assign(m_leftSize, -1);
    m_matchRight.assign(m_rightSize, -1);
}

/**
 * @brief 添加一条边(left -> right)
 * @param left 左侧顶点索引
 * @param right 右侧顶点索引
 */
void BipartiteGraph::addEdge(int left, int right)
{
    if (left < 0 || left >= m_leftSize) return;
    if (right < 0 || right >= m_rightSize) return;

    /* 避免重复边 */
    auto& neighbors = m_adj[left];
    if (!neighbors.contains(right)) {
        neighbors.append(right);
    }
}

/** @brief 清除所有边 */
void BipartiteGraph::clearEdges()
{
    m_adj.clear();
    m_matchLeft.assign(m_leftSize, -1);
    m_matchRight.assign(m_rightSize, -1);
}

// ── Hopcroft-Karp 最大匹配 ──

/**
 * @brief BFS构建层次图
 * @param dist 距离数组(输出)
 * @return 是否存在从空闲左顶点到空闲右顶点的增广路径
 *
 * BFS从所有未匹配的左顶点出发，构建交替路径层次图。
 */
bool BipartiteGraph::bfs(QVector<int>& dist)
{
    dist.assign(m_leftSize, kInf);
    std::queue<int> q;

    /* 将所有未匹配左顶点入队 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (m_matchLeft[u] == -1) {
            dist[u] = 0;
            q.push(u);
        }
    }

    bool found = false;
    while (!q.empty()) {
        int u = q.front();
        q.pop();

        auto it = m_adj.constFind(u);
        if (it == m_adj.constEnd()) continue;

        for (int v : it.value()) {
            int mu = m_matchRight[v];
            if (mu == -1) {
                /* 到达未匹配右顶点 → 存在增广路径 */
                found = true;
            } else if (dist[mu] == kInf) {
                /* 经过已匹配右顶点，继续BFS */
                dist[mu] = dist[u] + 1;
                q.push(mu);
            }
        }
    }
    return found;
}

/**
 * @brief DFS沿层次图寻找增广路径
 * @param u 当前左顶点
 * @param dist 距离数组
 * @return 是否找到增广路径
 */
bool BipartiteGraph::dfs(int u, QVector<int>& dist)
{
    auto it = m_adj.constFind(u);
    if (it == m_adj.constEnd()) return false;

    for (int v : it.value()) {
        int mu = m_matchRight[v];
        if (mu == -1 || (dist[mu] == dist[u] + 1 && dfs(mu, dist))) {
            /* 找到增广路径, 更新匹配 */
            m_matchLeft[u] = v;
            m_matchRight[v] = u;
            return true;
        }
    }
    dist[u] = kInf;
    return false;
}

/**
 * @brief 使用Hopcroft-Karp算法求最大匹配
 * @return 匹配结果(含匹配对、大小、迭代次数)
 */
BipartiteGraph::MatchingResult BipartiteGraph::maxMatching()
{
    MatchingResult result;
    QElapsedTimer timer;
    timer.start();

    m_matchLeft.assign(m_leftSize, -1);
    m_matchRight.assign(m_rightSize, -1);

    QVector<int> dist(m_leftSize, kInf);
    int matchSize = 0;
    int bfsCount = 0;

    /* Hopcroft-Karp主循环 */
    while (bfs(dist)) {
        ++bfsCount;
        for (int u = 0; u < m_leftSize; ++u) {
            if (m_matchLeft[u] == -1 && dfs(u, dist)) {
                ++matchSize;
            }
        }
    }

    /* 收集匹配对 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (m_matchLeft[u] != -1) {
            result.pairs.append({u, m_matchLeft[u]});
        }
    }

    result.size = matchSize;
    result.bfsIterations = bfsCount;
    result.elapsedMs = timer.elapsed();

    /* 更新统计 */
    ++m_stats.totalMatches;
    m_stats.totalMatchingsFound += matchSize;
    m_stats.totalVerticesProcessed += m_leftSize + m_rightSize;
    int edgeCount = 0;
    for (auto it = m_adj.constBegin(); it != m_adj.constEnd(); ++it) {
        edgeCount += it.value().size();
    }
    m_stats.totalEdgesProcessed += edgeCount;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalMatches);

    emit matchingCompleted(result);
    return result;
}

/**
 * @brief 获取最大匹配大小(不返回完整匹配结果)
 * @return 匹配数
 */
int BipartiteGraph::maxMatchingSize()
{
    auto result = maxMatching();
    return result.size;
}

// ── 顶点覆盖与独立集 ──

/**
 * @brief 计算最小顶点覆盖(Konig定理)
 * @param matching 当前匹配结果
 * @return 覆盖顶点列表(左侧用原始索引, 右侧用索引+偏移)
 *
 * Konig定理: 二部图最大匹配数 = 最小顶点覆盖数。
 * 算法: 从未匹配左顶点出发沿交替路径标记, 构造覆盖集。
 */
QVector<int> BipartiteGraph::minVertexCover(
    const MatchingResult& matching)
{
    QVector<int> cover;

    /* 标记已匹配的左/右顶点 */
    QSet<int> matchedLeft, matchedRight;
    for (const auto& p : matching.pairs) {
        matchedLeft.insert(p.first);
        matchedRight.insert(p.second);
    }

    /* 从未匹配左顶点BFS沿交替路径标记 */
    QSet<int> visitedLeft, visitedRight;
    std::queue<int> q;
    for (int u = 0; u < m_leftSize; ++u) {
        if (!matchedLeft.contains(u)) {
            q.push(u);
            visitedLeft.insert(u);
        }
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        auto it = m_adj.constFind(u);
        if (it == m_adj.constEnd()) continue;

        for (int v : it.value()) {
            if (!visitedRight.contains(v)) {
                visitedRight.insert(v);
                int mu = m_matchRight[v];
                if (mu != -1 && !visitedLeft.contains(mu)) {
                    visitedLeft.insert(mu);
                    q.push(mu);
                }
            }
        }
    }

    /* 最小覆盖: 未访问的匹配左顶点 + 访问过的右顶点 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (matchedLeft.contains(u) && !visitedLeft.contains(u)) {
            cover.append(u);
        }
    }
    for (int v = 0; v < m_rightSize; ++v) {
        if (visitedRight.contains(v)) {
            cover.append(v + m_leftSize);
        }
    }

    return cover;
}

/**
 * @brief 计算最大独立集(补集)
 * @param vertexCover 最小顶点覆盖
 * @return 独立集顶点列表
 */
QVector<int> BipartiteGraph::maxIndependentSet(
    const QVector<int>& vertexCover)
{
    QSet<int> coverSet;
    for (int v : vertexCover) coverSet.insert(v);

    QVector<int> indep;
    for (int u = 0; u < m_leftSize; ++u) {
        if (!coverSet.contains(u)) indep.append(u);
    }
    for (int v = 0; v < m_rightSize; ++v) {
        if (!coverSet.contains(v + m_leftSize)) indep.append(v + m_leftSize);
    }
    return indep;
}

// ── 辅助 ──

/** @brief 获取邻接表 */
const QMap<int, QVector<int>>& BipartiteGraph::adjacencyList() const
{
    return m_adj;
}

/** @brief 重置统计 */
void BipartiteGraph::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
