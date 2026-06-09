/**
 * @file VertexCover8.cpp
 * @brief VertexCover8 实现
 *
 * 实现顶点覆盖：2-近似极大匹配与度数折叠分支归约。
 */

#include "utils/graph258/VertexCover8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover8::VertexCover8(QObject *parent) : QObject(parent) {}
VertexCover8::~VertexCover8() = default;

/* ---- Configuration ---- */

void VertexCover8::setGraph(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_n = numVertices;
    m_edges = edges;
    m_adj.resize(m_n);
    for (auto& a : m_adj) a.clear();

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n && e.second >= 0 && e.second < m_n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }
}

/* ---- Greedy maximal matching ---- */

QVector<QPair<int, int>> VertexCover8::maximalMatching() const
{
    QVector<QPair<int, int>> matching;
    QVector<bool> matched(m_n, false);

    for (const auto& e : m_edges) {
        if (!matched[e.first] && !matched[e.second]) {
            matching.append(e);
            matched[e.first] = true;
            matched[e.second] = true;
        }
    }
    return matching;
}

/* ---- Effective degree ---- */

int VertexCover8::effectiveDegree(int v, const QVector<bool>& removed) const
{
    int deg = 0;
    for (int u : m_adj[v])
        if (!removed[u]) deg++;
    return deg;
}

/* ---- Count remaining edges ---- */

int VertexCover8::countEdges(const QVector<bool>& removed) const
{
    int count = 0;
    for (const auto& e : m_edges)
        if (!removed[e.first] && !removed[e.second]) count++;
    return count;
}

/* ---- Select branching vertex (highest degree) ---- */

int VertexCover8::selectBranch(const QVector<bool>& removed) const
{
    int bestV = -1, bestDeg = 0;
    for (int v = 0; v < m_n; ++v) {
        if (removed[v]) continue;
        int d = effectiveDegree(v, removed);
        if (d > bestDeg) { bestDeg = d; bestV = v; }
    }
    return bestV;
}

/* ---- Fold degree-0 and degree-1 vertices ---- */

int VertexCover8::foldVertices(QVector<bool>& inCover, QVector<bool>& removed,
                                 QVector<int>& cover)
{
    int folded = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        for (int v = 0; v < m_n; ++v) {
            if (removed[v] || inCover[v]) continue;
            int deg = effectiveDegree(v, removed);
            // Degree-0: isolated vertex, remove without adding to cover
            if (deg == 0) {
                removed[v] = true;
                changed = true;
                folded++;
                continue;
            }
            // Degree-1: include the neighbor instead (folding rule)
            if (deg == 1) {
                int neighbor = -1;
                for (int u : m_adj[v]) {
                    if (!removed[u]) { neighbor = u; break; }
                }
                if (neighbor >= 0) {
                    // Include neighbor in cover, remove both
                    inCover[neighbor] = true;
                    cover.append(neighbor);
                    removed[v] = true;
                    removed[neighbor] = true;
                    changed = true;
                    folded++;
                }
            }
        }
    }
    return folded;
}

/* ---- Branch-and-reduce recursive search ---- */

void VertexCover8::branchReduce(QVector<int>& cover, QVector<bool>& inCover,
                                  QVector<bool>& removed, int depth)
{
    // Fold easy vertices first
    foldVertices(inCover, removed, cover);

    int remaining = countEdges(removed);
    if (remaining == 0) {
        // All edges covered: check if this is better
        if (cover.size() < m_bestSize) {
            m_bestSize = cover.size();
            m_bestCover = cover;
        }
        return;
    }

    // Pruning: current cover + remaining edges > best
    if (cover.size() >= m_bestSize) return;

    // Select highest-degree vertex for branching
    int v = selectBranch(removed);
    if (v < 0) return;

    m_stats.numBranches++;

    // Branch 1: include vertex v in the cover
    {
        QVector<int> coverCopy = cover;
        QVector<bool> inCoverCopy = inCover;
        QVector<bool> removedCopy = removed;

        inCoverCopy[v] = true;
        coverCopy.append(v);
        removedCopy[v] = true;

        branchReduce(coverCopy, inCoverCopy, removedCopy, depth + 1);
    }

    // Branch 2: exclude v, include all its neighbors instead
    {
        QVector<int> coverCopy = cover;
        QVector<bool> inCoverCopy = inCover;
        QVector<bool> removedCopy = removed;

        removedCopy[v] = true;
        for (int u : m_adj[v]) {
            if (!removedCopy[u]) {
                inCoverCopy[u] = true;
                coverCopy.append(u);
                removedCopy[u] = true;
            }
        }

        branchReduce(coverCopy, inCoverCopy, removedCopy, depth + 1);
    }
}

/* ---- 2-Approximation ---- */

QVector<int> VertexCover8::approximate()
{
    QElapsedTimer timer;
    timer.start();

    auto matching = maximalMatching();
    QVector<int> cover;
    cover.reserve(matching.size() * 2);

    for (const auto& e : matching) {
        cover.append(e.first);
        cover.append(e.second);
    }

    m_stats.approxCoverSize = cover.size();
    m_stats.coverSize = cover.size();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coverComputed(cover.size(), false, timer.elapsed());
    return cover;
}

/* ---- Exact via branch-and-reduce ---- */

QVector<int> VertexCover8::exact()
{
    QElapsedTimer timer;
    timer.start();

    // Start with 2-approximation as upper bound
    auto approxCover = approximate();
    m_bestCover = approxCover;
    m_bestSize = approxCover.size();
    m_stats.numBranches = 0;
    m_stats.numFolds = 0;

    QVector<int> cover;
    QVector<bool> inCover(m_n, false);
    QVector<bool> removed(m_n, false);

    branchReduce(cover, inCover, removed, 0);

    m_stats.coverSize = m_bestSize;
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coverComputed(m_bestSize, true, timer.elapsed());
    return m_bestCover;
}

/* ---- Cover size accessor ---- */

int VertexCover8::coverSize() const { return m_stats.coverSize; }

/* ---- Reset ---- */

void VertexCover8::resetStatistics()
{
    m_adj.clear(); m_edges.clear(); m_bestCover.clear();
    m_bestSize = 0; m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
