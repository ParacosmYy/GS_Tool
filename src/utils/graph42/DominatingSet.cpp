/**
 * @file DominatingSet.cpp
 * @brief 支配集求解实现 — 贪心近似/单位圆图/连通支配集/最小权重
 */

#include "utils/graph42/DominatingSet.h"

#include <QtMath>
#include <algorithm>
#include <queue>
#include <vector>

DominatingSet::DominatingSet(QObject* parent)
    : QObject(parent)
{
}

QVector<int> DominatingSet::greedyDominatingSet(const AdjList& adj)
{
    m_timing.start();
    ++m_stats.totalSolves;
    int n = adj.size();
    if (n == 0) return {};

    QVector<bool> dominated(n, false);
    QVector<int> result;
    int undominated = n;

    while (undominated > 0) {
        ++m_stats.totalIterations;
        /* 选覆盖最多未覆盖顶点的顶点 */
        int bestV = -1;
        int bestCover = 0;
        for (int v = 0; v < n; ++v) {
            int cover = 0;
            if (!dominated[v]) ++cover;
            for (int nb : adj[v]) {
                if (nb >= 0 && nb < n && !dominated[nb]) ++cover;
            }
            if (cover > bestCover) { bestCover = cover; bestV = v; }
        }
        if (bestV < 0) break;
        result.append(bestV);
        if (!dominated[bestV]) { dominated[bestV] = true; --undominated; }
        for (int nb : adj[bestV]) {
            if (nb >= 0 && nb < n && !dominated[nb]) {
                dominated[nb] = true;
                --undominated;
            }
        }
    }
    m_stats.totalDominatingsetSize += result.size();
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;
    emit solveComplete(result.size(), n, m_stats.avgProcessingTimeMs);
    return result;
}

QVector<int> DominatingSet::unitDiskDominatingSet(const QVector<double>& positions,
                                                    double radius)
{
    m_timing.start();
    ++m_stats.totalSolves;
    int n = positions.size() / 2;
    if (n == 0) return {};
    AdjList adj = buildUnitDiskGraph(positions, radius);
    /* 单位圆图的贪心2-近似 */
    QVector<int> result = greedyDominatingSet(adj);
    /* 重置额外累加的统计(避免双重计数) */
    --m_stats.totalSolves;
    m_stats.totalSolves++;  // 保持只算一次
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;
    emit solveComplete(result.size(), n, m_stats.avgProcessingTimeMs);
    return result;
}

QVector<int> DominatingSet::connectedDominatingSet(const AdjList& adj)
{
    m_timing.start();
    ++m_stats.totalSolves;
    int n = adj.size();
    if (n == 0) return {};

    /* 先求贪心支配集 */
    QVector<bool> dominated(n, false);
    QVector<int> domSet;
    int undominated = n;
    while (undominated > 0) {
        ++m_stats.totalIterations;
        int bestV = -1, bestCover = 0;
        for (int v = 0; v < n; ++v) {
            int cover = 0;
            if (!dominated[v]) ++cover;
            for (int nb : adj[v])
                if (nb >= 0 && nb < n && !dominated[nb]) ++cover;
            if (cover > bestCover) { bestCover = cover; bestV = v; }
        }
        if (bestV < 0) break;
        domSet.append(bestV);
        if (!dominated[bestV]) { dominated[bestV] = true; --undominated; }
        for (int nb : adj[bestV])
            if (nb >= 0 && nb < n && !dominated[nb]) { dominated[nb] = true; --undominated; }
    }
    /* 连通化: 通过Steiner节点连接支配集 */
    QVector<int> connected = makeConnected(adj, domSet);
    m_stats.totalDominatingsetSize += connected.size();
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;
    emit solveComplete(connected.size(), n, m_stats.avgProcessingTimeMs);
    return connected;
}

QVector<int> DominatingSet::weightedDominatingSet(const AdjList& adj,
                                                    const QVector<double>& weights)
{
    m_timing.start();
    ++m_stats.totalSolves;
    int n = adj.size();
    if (n == 0) return {};
    QVector<bool> dominated(n, false);
    QVector<int> result;
    int undominated = n;

    while (undominated > 0) {
        ++m_stats.totalIterations;
        int bestV = -1;
        double bestRatio = 1e30;
        for (int v = 0; v < n; ++v) {
            if (dominated[v] && result.contains(v)) continue;
            int cover = 0;
            if (!dominated[v]) ++cover;
            for (int nb : adj[v])
                if (nb >= 0 && nb < n && !dominated[nb]) ++cover;
            if (cover == 0) continue;
            double w = (v < weights.size()) ? qMax(weights[v], 1e-15) : 1.0;
            double ratio = w / cover;
            if (ratio < bestRatio) { bestRatio = ratio; bestV = v; }
        }
        if (bestV < 0) break;
        result.append(bestV);
        if (!dominated[bestV]) { dominated[bestV] = true; --undominated; }
        for (int nb : adj[bestV])
            if (nb >= 0 && nb < n && !dominated[nb]) { dominated[nb] = true; --undominated; }
    }
    m_stats.totalDominatingsetSize += result.size();
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;
    emit solveComplete(result.size(), n, m_stats.avgProcessingTimeMs);
    return result;
}

bool DominatingSet::isDominating(const AdjList& adj, const QVector<int>& candidate) const
{
    int n = adj.size();
    QVector<bool> covered(n, false);
    QSet<int> candSet(candidate.begin(), candidate.end());
    for (int v : candidate) {
        if (v >= 0 && v < n) covered[v] = true;
        for (int nb : adj[v])
            if (nb >= 0 && nb < n) covered[nb] = true;
    }
    for (int i = 0; i < n; ++i)
        if (!covered[i]) return false;
    return true;
}

bool DominatingSet::isConnected(const AdjList& adj, const QVector<int>& candidate) const
{
    if (candidate.size() <= 1) return true;
    QSet<int> candSet(candidate.begin(), candidate.end());
    QVector<bool> visited(adj.size(), false);
    std::queue<int> q;
    q.push(candidate[0]);
    visited[candidate[0]] = true;
    int count = 1;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int nb : adj[u]) {
            if (candSet.contains(nb) && !visited[nb]) {
                visited[nb] = true;
                q.push(nb);
                ++count;
            }
        }
    }
    return count == candidate.size();
}

QVector<int> DominatingSet::bfsPath(const AdjList& adj, int src, int dst) const
{
    int n = adj.size();
    QVector<int> parent(n, -1);
    QVector<bool> visited(n, false);
    std::queue<int> q;
    q.push(src);
    visited[src] = true;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == dst) break;
        for (int nb : adj[u]) {
            if (nb >= 0 && nb < n && !visited[nb]) {
                visited[nb] = true;
                parent[nb] = u;
                q.push(nb);
            }
        }
    }
    QVector<int> path;
    if (!visited[dst]) return path;
    for (int v = dst; v != -1; v = parent[v])
        path.prepend(v);
    return path;
}

QVector<int> DominatingSet::makeConnected(const AdjList& adj, const QVector<int>& domSet)
{
    if (domSet.size() <= 1) return domSet;
    QSet<int> inSet(domSet.begin(), domSet.end());
    QVector<int> result = domSet;
    /* 连接不连通的支配集成员 */
    QSet<int> connected;
    QSet<int> remaining = inSet;
    connected.insert(domSet[0]);
    remaining.remove(domSet[0]);

    while (!remaining.isEmpty()) {
        /* 找到连接集合中的某点到remaining中某点的最短路径 */
        int bestPathLen = 1 << 30;
        QVector<int> bestPath;
        for (int src : connected) {
            for (int dst : remaining) {
                QVector<int> path = bfsPath(adj, src, dst);
                if (!path.isEmpty() && path.size() < bestPathLen) {
                    bestPathLen = path.size();
                    bestPath = path;
                }
            }
            if (bestPathLen <= 3) break;
        }
        if (bestPath.isEmpty()) break;
        /* 添加路径上的中间节点 */
        for (int v : bestPath) {
            if (!connected.contains(v)) {
                connected.insert(v);
                if (!inSet.contains(v)) {
                    result.append(v);
                    inSet.insert(v);
                }
                remaining.remove(v);
            }
        }
    }
    return result;
}

DominatingSet::AdjList DominatingSet::buildUnitDiskGraph(const QVector<double>& positions,
                                                          double radius) const
{
    int n = positions.size() / 2;
    AdjList adj(n);
    double r2 = radius * radius;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dx = positions[2 * i] - positions[2 * j];
            double dy = positions[2 * i + 1] - positions[2 * j + 1];
            if (dx * dx + dy * dy <= r2) {
                adj[i].append(j);
                adj[j].append(i);
            }
        }
    }
    return adj;
}

void DominatingSet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
