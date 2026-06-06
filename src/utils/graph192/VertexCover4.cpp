/**
 * @file VertexCover4.cpp
 * @brief VertexCover4 实现
 *
 * 实现最小顶点覆盖：分支约减、冠分解、核化技术。
 */

#include "utils/graph192/VertexCover4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

VertexCover4::VertexCover4(QObject *parent) : QObject(parent) {}
VertexCover4::~VertexCover4() = default;

/* ---- Set graph ---- */

void VertexCover4::setGraph(const QVector<QVector<int>>& adj)
{
    m_adj = adj;
    m_n = adj.size();
}

/* ---- Count remaining edges ---- */

int VertexCover4::countEdges(const QVector<bool>& removed) const
{
    int count = 0;
    for (int u = 0; u < m_n; ++u) {
        if (removed[u]) continue;
        for (int v : m_adj[u]) {
            if (v > u && !removed[v]) count++;
        }
    }
    return count;
}

/* ---- Find vertex with maximum degree ---- */

int VertexCover4::maxDegreeVertex(const QVector<bool>& removed) const
{
    int best = -1, bestDeg = 0;
    for (int u = 0; u < m_n; ++u) {
        if (removed[u]) continue;
        int deg = 0;
        for (int v : m_adj[u])
            if (!removed[v]) deg++;
        if (deg > bestDeg) { bestDeg = deg; best = u; }
    }
    return best;
}

/* ---- LP-based lower bound (half of remaining edges) ---- */

int VertexCover4::lowerBound() const
{
    QVector<bool> removed(m_n, false);
    int edges = countEdges(removed);
    return (edges + 1) / 2;
}

/* ---- Kernelization: remove degree-0/1 vertices ---- */

int VertexCover4::kernelize(QVector<int>& cover, QVector<bool>& removed)
{
    int reduced = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        for (int u = 0; u < m_n; ++u) {
            if (removed[u]) continue;
            int deg = 0, neighbor = -1;
            for (int v : m_adj[u]) {
                if (!removed[v]) { deg++; neighbor = v; }
            }
            if (deg == 0) {
                /* Isolated vertex: remove */
                removed[u] = true;
                reduced++;
                changed = true;
            } else if (deg == 1) {
                /* Degree-1: must include neighbor in cover */
                if (!removed[neighbor]) {
                    cover.append(neighbor);
                    removed[neighbor] = true;
                    reduced++;
                }
                removed[u] = true;
                reduced++;
                changed = true;
            }
        }
    }
    return reduced;
}

/* ---- Crown decomposition: find crown (I, H, C) ---- */

int VertexCover4::crownDecompose(QVector<bool>& inCover,
                                   QVector<bool>& removed)
{
    /* Find maximal matching to identify crown structure */
    QVector<int> match(m_n, -1);
    int crownEdges = 0;

    for (int u = 0; u < m_n; ++u) {
        if (removed[u] || match[u] >= 0) continue;
        for (int v : m_adj[u]) {
            if (!removed[v] && match[v] < 0) {
                match[u] = v;
                match[v] = u;
                break;
            }
        }
    }

    /* Find unmatched vertices -> potential crown head */
    QVector<bool> inMatch(m_n, false);
    for (int u = 0; u < m_n; ++u)
        if (match[u] >= 0) inMatch[u] = true;

    /* Crown: unmatched vertices whose neighbors are all matched */
    for (int u = 0; u < m_n; ++u) {
        if (removed[u] || inMatch[u]) continue;
        bool allMatched = true;
        for (int v : m_adj[u]) {
            if (!removed[v] && !inMatch[v]) { allMatched = false; break; }
        }
        if (allMatched && !m_adj[u].isEmpty()) {
            /* Include the matched neighbor (head) in cover */
            for (int v : m_adj[u]) {
                if (!removed[v] && !inCover[v]) {
                    inCover[v] = true;
                    removed[v] = true;
                    crownEdges++;
                }
            }
            removed[u] = true;
        }
    }
    return crownEdges;
}

/* ---- Branch-and-reduce main recursion ---- */

void VertexCover4::branchReduce(QVector<bool> inCover,
                                  QVector<bool> removed, int currentSize)
{
    /* Prune if already worse than best */
    if (currentSize >= m_bestSize) return;

    /* Kernelize */
    QVector<int> kernelCover;
    kernelize(kernelCover, removed);
    currentSize += kernelCover.size();
    if (currentSize >= m_bestSize) return;

    /* Crown decomposition */
    int crownAdded = crownDecompose(inCover, removed);
    currentSize += crownAdded;
    if (currentSize >= m_bestSize) return;

    /* Check if all edges covered */
    int remaining = countEdges(removed);
    if (remaining == 0) {
        if (currentSize < m_bestSize) {
            m_bestSize = currentSize;
            m_bestCover.clear();
            for (int u = 0; u < m_n; ++u)
                if (inCover[u] || (!removed[u]))
                    m_bestCover.append(u);
            /* Keep only cover vertices */
            QVector<int> finalCover;
            for (int u = 0; u < m_n; ++u)
                if (inCover[u]) finalCover.append(u);
            for (int v : kernelCover) finalCover.append(v);
            m_bestCover = finalCover;
        }
        return;
    }

    /* LP lower bound pruning */
    int lb = currentSize + (remaining + 1) / 2;
    if (lb >= m_bestSize) return;

    /* Branch on highest degree vertex */
    int branch = maxDegreeVertex(removed);
    if (branch < 0) return;

    /* Branch 1: include branch vertex in cover */
    QVector<bool> cover1 = inCover;
    QVector<bool> rem1 = removed;
    cover1[branch] = true;
    rem1[branch] = true;
    branchReduce(cover1, rem1, currentSize + 1);

    /* Branch 2: exclude branch, include all neighbors */
    QVector<bool> cover2 = inCover;
    QVector<bool> rem2 = removed;
    rem2[branch] = true;
    int nbrs = 0;
    for (int v : m_adj[branch]) {
        if (!rem2[v]) {
            cover2[v] = true;
            rem2[v] = true;
            nbrs++;
        }
    }
    branchReduce(cover2, rem2, currentSize + nbrs);
}

/* ---- Main solve ---- */

QVector<int> VertexCover4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    int edgeCount = 0;
    for (int u = 0; u < m_n; ++u)
        for (int v : m_adj[u])
            if (v > u) edgeCount++;

    /* Initialize best with trivial upper bound (all vertices) */
    m_bestSize = m_n;
    m_bestCover.clear();
    for (int i = 0; i < m_n; ++i) m_bestCover.append(i);

    QVector<bool> inCover(m_n, false);
    QVector<bool> removed(m_n, false);

    branchReduce(inCover, removed, 0);

    /* Remove duplicates from cover */
    QVector<bool> inFinal(m_n, false);
    QVector<int> result;
    for (int v : m_bestCover) {
        if (v >= 0 && v < m_n && !inFinal[v]) {
            inFinal[v] = true;
            result.append(v);
        }
    }

    m_stats.totalSolves++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = edgeCount;
    m_stats.coverSize = result.size();
    m_stats.kernelSize = m_n - result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(result.size(), m_stats.kernelSize);
    return result;
}

/* ---- Verify cover ---- */

bool VertexCover4::verifyCover(const QVector<int>& cover) const
{
    QVector<bool> inC(m_n, false);
    for (int v : cover) {
        if (v < 0 || v >= m_n) return false;
        inC[v] = true;
    }
    for (int u = 0; u < m_n; ++u)
        for (int v : m_adj[u])
            if (v > u && !inC[u] && !inC[v]) return false;
    return true;
}

/* ---- Reset ---- */

void VertexCover4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
