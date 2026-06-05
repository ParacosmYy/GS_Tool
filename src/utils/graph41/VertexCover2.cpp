/**
 * @file VertexCover2.cpp
 * @brief 顶点覆盖增强实现 — 2-近似/分支定界/核化简/Buss核化
 */

#include "utils/graph41/VertexCover2.h"

#include <QElapsedTimer>
#include <algorithm>
#include <unordered_set>

/** @brief 构造函数 @param parent 父对象 */
VertexCover2::VertexCover2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 构建邻接表图 @param edges 边列表 @param vertexCount 顶点数 */
void VertexCover2::buildGraph(const QList<QPair<int, int>>& edges,
                               int vertexCount)
{
    m_vertexCount = vertexCount;
    m_edges = edges;
    m_adjList.assign(vertexCount, QVector<int>());

    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < vertexCount
            && edge.second >= 0 && edge.second < vertexCount) {
            m_adjList[edge.first].append(edge.second);
            m_adjList[edge.second].append(edge.first);
        }
    }
}

/** @brief 求解顶点覆盖 @param algo 算法 @param k 参数 @return 解 */
VertexCover2::Solution VertexCover2::solve(Algorithm algo, int k)
{
    QElapsedTimer timer;
    timer.start();

    Solution sol;
    switch (algo) {
    case Algorithm::Greedy2Approx:
        sol = greedy2Approx();
        break;
    case Algorithm::BranchAndBound:
        sol = branchAndBound();
        break;
    case Algorithm::Kernelization:
        sol = kernelization(k);
        break;
    case Algorithm::BussKernel:
        sol = bussKernel(k);
        break;
    }

    sol.solveTimeMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.totalVerticesProcessed += m_vertexCount;
    m_timeSum += sol.solveTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);
    if (sol.isOptimal) m_stats.totalOptimalSolves++;

    emit solveComplete(sol.vertices.size(), sol.isOptimal);
    return sol;
}

/** @brief 验证解是否为合法顶点覆盖 @param cover 顶点集合 @return 是否合法 */
bool VertexCover2::verifyCover(const QVector<int>& cover) const
{
    std::unordered_set<int> coverSet(cover.begin(), cover.end());
    for (const auto& edge : m_edges) {
        if (coverSet.find(edge.first) == coverSet.end()
            && coverSet.find(edge.second) == coverSet.end()) {
            return false;
        }
    }
    return true;
}

/** @brief 获取图信息 @return (顶点数, 边数) */
QPair<int, int> VertexCover2::graphInfo() const
{
    return {m_vertexCount, m_edges.size()};
}

/** @brief 贪心2-近似 @return 近似解 */
VertexCover2::Solution VertexCover2::greedy2Approx()
{
    Solution sol;
    sol.isOptimal = false;

    std::unordered_set<int> covered;
    QVector<QPair<int, int>> remaining = m_edges.toVector();
    QVector<int> cover;

    /* 每次取一条未覆盖的边，将两端都加入覆盖 */
    for (const auto& edge : remaining) {
        if (covered.find(edge.first) != covered.end()
            || covered.find(edge.second) != covered.end()) {
            continue;
        }
        cover.append(edge.first);
        cover.append(edge.second);
        covered.insert(edge.first);
        covered.insert(edge.second);
    }

    sol.vertices = cover;
    return sol;
}

/** @brief 分支定界精确求解 @return 最优解 */
VertexCover2::Solution VertexCover2::branchAndBound()
{
    Solution sol;
    sol.isOptimal = true;

    QVector<QPair<int, int>> edges = m_edges.toVector();
    QVector<int> currentCover;
    QVector<int> bestCover;

    /* 初始上界: 贪心解 */
    Solution greedy = greedy2Approx();
    bestCover = greedy.vertices;
    int bestSize = bestCover.size();

    bbSearch(edges, currentCover, bestCover, bestSize);

    sol.vertices = bestCover;
    return sol;
}

/** @brief 分支定界递归搜索 @param remainingEdges 剩余边 @param currentCover 当前覆盖 @param bestCover 最优覆盖 @param k 上界 */
void VertexCover2::bbSearch(const QVector<QPair<int, int>>& remainingEdges,
                             QVector<int>& currentCover,
                             QVector<int>& bestCover, int k)
{
    /* 剪枝: 超过当前最优 */
    if (currentCover.size() >= k) return;

    /* 无剩余边: 更新最优 */
    if (remainingEdges.isEmpty()) {
        if (currentCover.size() < bestCover.size()) {
            bestCover = currentCover;
        }
        return;
    }

    /* 取第一条未覆盖的边(u,v) */
    auto edge = remainingEdges.first();
    std::unordered_set<int> curSet(currentCover.begin(), currentCover.end());

    /* 找到第一条未被覆盖的边 */
    int targetEdge = -1;
    for (int i = 0; i < remainingEdges.size(); ++i) {
        if (curSet.find(remainingEdges[i].first) == curSet.end()
            && curSet.find(remainingEdges[i].second) == curSet.end()) {
            targetEdge = i;
            break;
        }
    }

    if (targetEdge < 0) {
        /* 所有边已覆盖 */
        if (currentCover.size() < bestCover.size()) {
            bestCover = currentCover;
        }
        return;
    }

    int u = remainingEdges[targetEdge].first;
    int v = remainingEdges[targetEdge].second;

    /* 分支: 选u */
    QVector<QPair<int, int>> edgesAfterU;
    for (int i = 0; i < remainingEdges.size(); ++i) {
        if (remainingEdges[i].first == u || remainingEdges[i].second == u) {
            continue;
        }
        edgesAfterU.append(remainingEdges[i]);
    }
    currentCover.append(u);
    bbSearch(edgesAfterU, currentCover, bestCover, bestCover.size());
    currentCover.removeLast();

    /* 分支: 选v */
    QVector<QPair<int, int>> edgesAfterV;
    for (int i = 0; i < remainingEdges.size(); ++i) {
        if (remainingEdges[i].first == v || remainingEdges[i].second == v) {
            continue;
        }
        edgesAfterV.append(remainingEdges[i]);
    }
    currentCover.append(v);
    bbSearch(edgesAfterV, currentCover, bestCover, bestCover.size());
    currentCover.removeLast();
}

/** @brief 核化简+参数化求解 @param k 参数 @return 解 */
VertexCover2::Solution VertexCover2::kernelization(int k)
{
    Solution sol;
    if (k < 0) k = m_vertexCount;
    sol.isOptimal = true;

    QVector<QPair<int, int>> edges = m_edges.toVector();
    QVector<int> forcedCover;

    reduceGraph(edges, forcedCover, k);

    if (edges.isEmpty()) {
        sol.vertices = forcedCover;
        return sol;
    }

    /* 核化后若剩余边不超过k^2, 继续分支定界 */
    if (static_cast<int>(edges.size()) <= k * k) {
        QVector<int> currentCover = forcedCover;
        QVector<int> bestCover = forcedCover;
        bestCover.resize(m_vertexCount);
        for (int i = 0; i < m_vertexCount; ++i) bestCover[i] = i;

        bbSearch(edges, currentCover, bestCover, bestCover.size());
        sol.vertices = bestCover;
    } else {
        /* 核化后仍然过大, k太小无法覆盖 */
        sol.vertices = forcedCover;
        sol.isOptimal = false;
    }

    return sol;
}

/** @brief Buss核化(度数过滤) @param k 参数 @return 解 */
VertexCover2::Solution VertexCover2::bussKernel(int k)
{
    Solution sol;
    if (k < 0) k = m_vertexCount;
    sol.isOptimal = false;

    QVector<int> cover;
    QVector<QPair<int, int>> edges = m_edges.toVector();
    int remainingK = k;

    /* 规则1: 度数>k的顶点必须选 */
    bool changed = true;
    while (changed) {
        changed = false;
        QVector<int> degree(m_vertexCount, 0);
        for (const auto& e : edges) {
            degree[e.first]++;
            degree[e.second]++;
        }
        for (int v = 0; v < m_vertexCount; ++v) {
            if (degree[v] > remainingK && remainingK > 0) {
                cover.append(v);
                --remainingK;
                /* 移除关联边 */
                edges.erase(std::remove_if(edges.begin(), edges.end(),
                    [v](const QPair<int, int>& e) {
                        return e.first == v || e.second == v;
                    }), edges.end());
                changed = true;
            }
        }
    }

    /* 规则2: 剩余边数 > k*remainingK 则无解 */
    if (static_cast<int>(edges.size()) > k * remainingK) {
        sol.vertices = cover;
        return sol;
    }

    /* 对剩余小图精确求解 */
    if (!edges.isEmpty()) {
        QVector<int> currentCover = cover;
        QVector<int> bestCover = cover;
        for (int i = 0; i < m_vertexCount; ++i) bestCover.append(i);

        bbSearch(edges, currentCover, bestCover, bestCover.size());
        sol.vertices = bestCover;
        sol.isOptimal = true;
    } else {
        sol.vertices = cover;
        sol.isOptimal = true;
    }

    return sol;
}

/** @brief 核化简: 移除度0/1顶点和高度顶点 */
void VertexCover2::reduceGraph(QVector<QPair<int, int>>& edges,
                                QVector<int>& cover, int k)
{
    bool changed = true;
    while (changed) {
        changed = false;
        QVector<int> degree(m_vertexCount, 0);
        for (const auto& e : edges) {
            degree[e.first]++;
            degree[e.second]++;
        }
        for (int v = 0; v < m_vertexCount; ++v) {
            if (degree[v] == 0) continue;
            if (degree[v] == 1) {
                /* 度1顶点: 选其邻居 */
                int neighbor = -1;
                for (const auto& e : edges) {
                    if (e.first == v) { neighbor = e.second; break; }
                    if (e.second == v) { neighbor = e.first; break; }
                }
                if (neighbor >= 0) {
                    cover.append(neighbor);
                    edges.erase(std::remove_if(edges.begin(), edges.end(),
                        [neighbor](const QPair<int, int>& e) {
                            return e.first == neighbor || e.second == neighbor;
                        }), edges.end());
                    changed = true;
                }
            } else if (degree[v] > k) {
                cover.append(v);
                edges.erase(std::remove_if(edges.begin(), edges.end(),
                    [v](const QPair<int, int>& e) {
                        return e.first == v || e.second == v;
                    }), edges.end());
                changed = true;
            }
        }
    }
}

/** @brief 重置统计 */
void VertexCover2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
