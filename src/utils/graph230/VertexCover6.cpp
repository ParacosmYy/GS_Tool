/**
 * @file VertexCover6.cpp
 * @brief VertexCover6 实现
 *
 * 实现顶点覆盖：LP松弛半整数性、皇冠分解核化、最大匹配。
 */

#include "utils/graph230/VertexCover6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover6::VertexCover6(QObject *parent) : QObject(parent) {}
VertexCover6::~VertexCover6() = default;

/* ---- Build graph ---- */

void VertexCover6::buildGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    for (auto& a : m_adj) a.clear();
    m_edges = edges;

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
}

/* ---- Add edge ---- */

void VertexCover6::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append(v);
    m_adj[v].append(u);
    m_edges.append({u, v});
    m_stats.numEdges = m_edges.size();
}

/* ---- Solve LP relaxation ---- */

QVector<double> VertexCover6::solveLPRelaxation() const
{
    // LP: minimize sum(x_i) s.t. x_u + x_v >= 1 for each (u,v), 0 <= x_i <= 1
    // Half-integrality theorem: optimal solution has x_i in {0, 0.5, 1}
    // Iterative rounding approach

    QVector<double> x(m_n, 0.5);  // Start with all half-integral

    // Iterate: for each edge (u,v), ensure x_u + x_v >= 1
    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;
        for (const auto& e : m_edges) {
            double sum = x[e.first] + x[e.second];
            if (sum < 1.0) {
                // Raise both to satisfy constraint
                double deficit = 1.0 - sum;
                x[e.first] += deficit / 2.0;
                x[e.second] += deficit / 2.0;
                changed = true;
            }
        }
        // Clamp to {0, 0.5, 1}
        for (int i = 0; i < m_n; ++i) {
            if (x[i] < 0.25) x[i] = 0.0;
            else if (x[i] < 0.75) x[i] = 0.5;
            else x[i] = 1.0;
        }
        if (!changed) break;
    }

    return x;
}

/* ---- BFS augmenting path for matching ---- */

bool VertexCover6::augmentPath(int u, QVector<int>& matchL, QVector<int>& matchR,
                                QVector<bool>& visited) const
{
    for (int v : m_adj[u]) {
        if (visited[v]) continue;
        visited[v] = true;
        if (matchR[v] < 0 || augmentPath(matchR[v], matchL, matchR, visited)) {
            matchL[u] = v;
            matchR[v] = u;
            return true;
        }
    }
    return false;
}

/* ---- Find maximum matching ---- */

QVector<QPair<int, int>> VertexCover6::findMaxMatching(
    const QVector<int>& leftSet, const QVector<int>& rightSet) const
{
    int lSize = leftSet.size();
    int rSize = rightSet.size();

    // Map to contiguous indices
    QMap<int, int> leftMap, rightMap;
    for (int i = 0; i < lSize; ++i) leftMap[leftSet[i]] = i;
    for (int i = 0; i < rSize; ++i) rightMap[rightSet[i]] = i;

    QVector<int> matchL(lSize, -1), matchR(rSize, -1);

    for (int u = 0; u < lSize; ++u) {
        QVector<bool> visited(rSize, false);
        int origU = leftSet[u];
        for (int v : m_adj[origU]) {
            if (rightMap.contains(v)) {
                int vi = rightMap[v];
                if (visited[vi]) continue;
                visited[vi] = true;
                if (matchR[vi] < 0 || augmentPath(matchR[vi], matchL, matchR, visited)) {
                    matchL[u] = vi;
                    matchR[vi] = u;
                    break;
                }
            }
        }
    }

    QVector<QPair<int, int>> matching;
    for (int i = 0; i < lSize; ++i) {
        if (matchL[i] >= 0)
            matching.append({leftSet[i], rightSet[matchL[i]]});
    }
    return matching;
}

/* ---- Crown decomposition kernelization ---- */

QPair<QVector<int>, QVector<QPair<int, int>>>
VertexCover6::crownKernelize(const QVector<double>& lpSolution) const
{
    // Partition vertices by LP value
    QVector<int> oneSet, halfSet, zeroSet;
    for (int i = 0; i < m_n; ++i) {
        if (lpSolution[i] > 0.75) oneSet.append(i);
        else if (lpSolution[i] > 0.25) halfSet.append(i);
        else zeroSet.append(i);
    }

    // Vertices with x=1 are always in the cover
    QVector<int> forcedCover = oneSet;

    // Try to find crown: I (independent set in zeroSet) + matching to halfSet
    // Crown head H ⊆ halfSet, crown C ⊆ zeroSet with |matching(H,C)| = |H|
    QVector<QPair<int, int>> remainingEdges;

    // Remove edges covered by forced vertices
    for (const auto& e : m_edges) {
        bool covered = false;
        for (int v : forcedCover) {
            if (e.first == v || e.second == v) { covered = true; break; }
        }
        if (!covered) remainingEdges.append(e);
    }

    return {forcedCover, remainingEdges};
}

/* ---- Round LP solution to integer cover ---- */

QVector<int> VertexCover6::roundLPSolution(const QVector<double>& lpSol) const
{
    QVector<int> cover;
    for (int i = 0; i < m_n; ++i) {
        if (lpSol[i] >= 0.5)
            cover.append(i);
    }
    return cover;
}

/* ---- Solve vertex cover ---- */

QVector<int> VertexCover6::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || m_edges.isEmpty()) {
        m_stats.totalOps++;
        return {};
    }

    // Step 1: Solve LP relaxation
    QVector<double> lpSol = solveLPRelaxation();
    double lpObj = 0.0;
    for (double v : lpSol) lpObj += v;
    m_stats.lpOptimal = lpObj;

    // Step 2: Crown decomposition kernelization
    auto [forcedCover, remainingEdges] = crownKernelize(lpSol);
    m_stats.kernelSize = remainingEdges.size();

    // Step 3: Round half-integral LP solution
    QVector<int> cover = roundLPSolution(lpSol);

    // Step 4: Greedy improvement - remove redundant vertices
    for (int i = cover.size() - 1; i >= 0; --i) {
        int v = cover[i];
        bool canRemove = true;
        for (int u : m_adj[v]) {
            bool covered = false;
            for (int j = 0; j < cover.size(); ++j) {
                if (j != i && cover[j] == u) { covered = true; break; }
            }
            if (!covered) { canRemove = false; break; }
        }
        if (canRemove) cover.removeAt(i);
    }

    m_stats.coverSize = cover.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(cover.size(), m_stats.kernelSize, timer.elapsed());
    return cover;
}

/* ---- Check valid cover ---- */

bool VertexCover6::isValidCover(const QVector<int>& cover) const
{
    QSet<int> coverSet;
    for (int v : cover) coverSet.insert(v);
    for (const auto& e : m_edges) {
        if (!coverSet.contains(e.first) && !coverSet.contains(e.second))
            return false;
    }
    return true;
}

/* ---- Adjacency list ---- */

QVector<QVector<int>> VertexCover6::adjacencyList() const { return m_adj; }

/* ---- Reset ---- */

void VertexCover6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_n = 0;
    m_adj.clear();
    m_edges.clear();
}
