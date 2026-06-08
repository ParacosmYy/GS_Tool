/**
 * @file GraphIsomorphism5.cpp
 * @brief GraphIsomorphism5 实现
 *
 * 实现图同构判定：个体化-细化搜索树、典范标记、无迹增强。
 */

#include "utils/graph233/GraphIsomorphism5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism5::GraphIsomorphism5(QObject *parent) : QObject(parent) {}
GraphIsomorphism5::~GraphIsomorphism5() = default;

/* ---- Count edges ---- */

int GraphIsomorphism5::countEdges(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    int count = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            count += adj[i][j];
    return count;
}

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism5::degreeSequence(
    const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            deg[i] += adj[i][j];
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

/* ---- Is discrete coloring ---- */

bool GraphIsomorphism5::isDiscrete(const QVector<int>& coloring) const
{
    for (int i = 0; i < coloring.size(); ++i)
        for (int j = i + 1; j < coloring.size(); ++j)
            if (coloring[i] == coloring[j]) return false;
    return true;
}

/* ---- First non-trivial color class ---- */

int GraphIsomorphism5::firstNonTrivialClass(const QVector<int>& coloring) const
{
    int n = coloring.size();
    for (int c = 0; c < n; ++c) {
        int count = 0;
        for (int i = 0; i < n; ++i)
            if (coloring[i] == c) count++;
        if (count > 1) return c;
    }
    return -1;
}

/* ---- Individualize vertex ---- */

QVector<int> GraphIsomorphism5::individualize(
    const QVector<int>& coloring, int vertex) const
{
    QVector<int> result = coloring;
    // Assign a new unique color to vertex
    int maxColor = 0;
    for (int c : result) maxColor = qMax(maxColor, c);
    result[vertex] = maxColor + 1;
    return result;
}

/* ---- Refine coloring via Weisfeiler-Lehman ---- */

QVector<int> GraphIsomorphism5::refine(const QVector<QVector<int>>& adj,
                                         QVector<int> coloring) const
{
    int n = adj.size();
    bool changed = true;
    int steps = 0;

    while (changed && steps < n) {
        changed = false;
        steps++;

        // Compute neighbor color multiset signatures
        QVector<QVector<int>> signatures(n);
        for (int v = 0; v < n; ++v) {
            QVector<int> neighborColors;
            for (int u = 0; u < n; ++u) {
                if (adj[v][u]) neighborColors.append(coloring[u]);
            }
            std::sort(neighborColors.begin(), neighborColors.end());
            // Signature = own color + neighbor colors
            signatures[v].append(coloring[v]);
            signatures[v].append(neighborColors);
        }

        // Re-color based on signatures
        QVector<int> newColoring(n, 0);
        int nextColor = 0;
        QVector<QVector<int>> seen;

        for (int v = 0; v < n; ++v) {
            int found = -1;
            for (int s = 0; s < seen.size(); ++s) {
                if (seen[s] == signatures[v]) { found = s; break; }
            }
            if (found >= 0) {
                newColoring[v] = found;
            } else {
                newColoring[v] = nextColor;
                seen.append(signatures[v]);
                nextColor++;
            }
        }

        if (newColoring != coloring) changed = true;
        coloring = newColoring;
    }
    return coloring;
}

/* ---- Canonical hash ---- */

quint64 GraphIsomorphism5::canonicalHash(const QVector<QVector<int>>& adj,
                                            const QVector<int>& labeling) const
{
    int n = adj.size();
    quint64 hash = 0x9E3779B97F4A7C15ULL;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (labeling[i] < labeling[j] && adj[i][j]) {
                hash ^= static_cast<quint64>(labeling[i] * 31 + labeling[j]) * 0x517CC1B727220A95ULL;
                hash = (hash << 17) | (hash >> 47);
            }
        }
    }
    return hash;
}

/* ---- Search tree with individualization-refinement ---- */

QVector<int> GraphIsomorphism5::searchTree(const QVector<QVector<int>>& adj,
                                              QVector<int> coloring,
                                              int depth, int maxDepth)
{
    coloring = refine(adj, coloring);
    m_stats.refinementSteps++;

    if (isDiscrete(coloring)) return coloring;
    if (depth >= maxDepth) return coloring;

    // Find first non-trivial color class and branch
    int colorClass = firstNonTrivialClass(coloring);
    if (colorClass < 0) return coloring;

    // Collect vertices in this color class
    QVector<int> vertices;
    for (int i = 0; i < coloring.size(); ++i)
        if (coloring[i] == colorClass) vertices.append(i);

    // Try individualizing first vertex in class (traceless: no backtracking)
    QVector<int> individualized = individualize(coloring, vertices[0]);
    return searchTree(adj, individualized, depth + 1, maxDepth);
}

/* ---- Canonical labeling ---- */

QVector<int> GraphIsomorphism5::canonicalLabeling(
    const QVector<QVector<int>>& adj)
{
    int n = adj.size();
    if (n == 0) return QVector<int>();

    // Initial coloring by degree
    QVector<int> coloring(n, 0);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j) deg += adj[i][j];
        coloring[i] = deg;
    }

    // Run search tree with individualization-refinement
    QVector<int> result = searchTree(adj, coloring, 0, n);

    // Convert to labeling (permutation)
    QVector<int> labeling(n);
    for (int i = 0; i < n; ++i) labeling[i] = result[i];
    return labeling;
}

/* ---- Isomorphic check ---- */

bool GraphIsomorphism5::isIsomorphic(const QVector<QVector<int>>& adjA,
                                       const QVector<QVector<int>>& adjB)
{
    QElapsedTimer timer;
    timer.start();

    int nA = adjA.size(), nB = adjB.size();
    bool result = false;

    // Quick rejects
    if (nA != nB) { result = false; }
    else if (countEdges(adjA) != countEdges(adjB)) { result = false; }
    else if (degreeSequence(adjA) != degreeSequence(adjB)) { result = false; }
    else {
        // Compute canonical labelings and compare hashes
        QVector<int> labelA = canonicalLabeling(adjA);
        QVector<int> labelB = canonicalLabeling(adjB);
        quint64 hashA = canonicalHash(adjA, labelA);
        quint64 hashB = canonicalHash(adjB, labelB);
        result = (hashA == hashB);
    }

    m_stats.numVertices = nA;
    m_stats.numEdges = countEdges(adjA);
    m_stats.lastResult = result;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit isomorphismChecked(nA, result, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
