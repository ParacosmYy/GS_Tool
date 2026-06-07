/**
 * @file GraphIsomorphism3.cpp
 * @brief GraphIsomorphism3 实现
 *
 * 实现图同构检测：个体化-精炼算法、规范标签、自同构群生成器。
 */

#include "utils/graph218/GraphIsomorphism3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphIsomorphism3::GraphIsomorphism3(QObject *parent) : QObject(parent) {}
GraphIsomorphism3::~GraphIsomorphism3() = default;

/* ---- Refine coloring ---- */

QVector<int> GraphIsomorphism3::refineColoring(const QVector<QVector<int>>& graph,
                                                 const QVector<int>& coloring) const
{
    int n = graph.size();
    if (n == 0) return coloring;

    QVector<int> current = coloring;
    bool changed = true;

    while (changed) {
        changed = false;
        // Count neighbor color distributions for each vertex
        QVector<QVector<int>> neighborColors(n);
        for (int i = 0; i < n; ++i) {
            QVector<int> colorProfile;
            for (int j = 0; j < n; ++j) {
                if (graph[i][j] != 0)
                    colorProfile.append(current[j]);
            }
            std::sort(colorProfile.begin(), colorProfile.end());
            neighborColors[i] = colorProfile;
        }

        // Assign new colors based on (current color, neighbor color profile)
        QVector<QPair<int, QVector<int>>> signatures(n);
        for (int i = 0; i < n; ++i)
            signatures[i] = {current[i], neighborColors[i]};

        // Stable sort to get new coloring
        QVector<int> indices(n);
        for (int i = 0; i < n; ++i) indices[i] = i;
        std::stable_sort(indices.begin(), indices.end(), [&](int a, int b) {
            if (signatures[a].first != signatures[b].first)
                return signatures[a].first < signatures[b].first;
            return signatures[a].second < signatures[b].second;
        });

        QVector<int> newColoring(n);
        int colorIdx = 0;
        newColoring[indices[0]] = colorIdx;
        for (int i = 1; i < n; ++i) {
            if (signatures[indices[i]] != signatures[indices[i - 1]]) colorIdx++;
            newColoring[indices[i]] = colorIdx;
        }

        if (newColoring != current) { current = newColoring; changed = true; }
    }
    return current;
}

/* ---- Individualize ---- */

QVector<int> GraphIsomorphism3::individualize(const QVector<int>& coloring, int vertex) const
{
    QVector<int> result = coloring;
    // Assign a unique new color to this vertex
    int maxColor = 0;
    for (int c : result) maxColor = qMax(maxColor, c);
    result[vertex] = maxColor + 1;
    return result;
}

/* ---- Search tree ---- */

void GraphIsomorphism3::searchTree(const QVector<QVector<int>>& graph,
                                     const QVector<int>& coloring,
                                     QVector<QVector<int>>& canonicalCandidates) const
{
    QVector<int> refined = refineColoring(graph, coloring);

    if (isDiscrete(refined)) {
        canonicalCandidates.append(refined);
        return;
    }

    // Find target color class to individualize
    int targetColor = targetColorClass(refined);
    QVector<int> candidates;
    for (int i = 0; i < refined.size(); ++i)
        if (refined[i] == targetColor) candidates.append(i);

    // Branch on each vertex in the target class
    for (int v : candidates) {
        QVector<int> individualized = individualize(refined, v);
        searchTree(graph, individualized, canonicalCandidates);
    }
}

/* ---- Canonical label ---- */

QVector<int> GraphIsomorphism3::canonicalLabel(const QVector<QVector<int>>& graph) const
{
    int n = graph.size();
    if (n == 0) return {};

    // Start with trivial coloring
    QVector<int> initial(n, 0);
    QVector<QVector<int>> candidates;
    searchTree(graph, initial, candidates);

    if (candidates.isEmpty()) return initial;

    // Select lexicographically smallest as canonical
    QVector<int> best = candidates[0];
    for (const auto& cand : candidates) {
        if (coloringLess(cand, best)) best = cand;
    }
    return best;
}

/* ---- Is isomorphic ---- */

bool GraphIsomorphism3::isIsomorphic(const QVector<QVector<int>>& g1, const QVector<QVector<int>>& g2)
{
    QElapsedTimer timer;
    timer.start();

    int n1 = g1.size(), n2 = g2.size();
    if (n1 != n2) return false;

    // Quick degree sequence check
    QVector<int> deg1(n1, 0), deg2(n2, 0);
    for (int i = 0; i < n1; ++i)
        for (int j = 0; j < n1; ++j) deg1[i] += g1[i][j];
    for (int i = 0; i < n2; ++i)
        for (int j = 0; j < n2; ++j) deg2[i] += g2[i][j];
    QVector<int> sorted1 = deg1, sorted2 = deg2;
    std::sort(sorted1.begin(), sorted1.end());
    std::sort(sorted2.begin(), sorted2.end());
    if (sorted1 != sorted2) return false;

    QVector<int> can1 = canonicalLabel(g1);
    QVector<int> can2 = canonicalLabel(g2);
    bool result = (can1 == can2);

    m_stats.totalChecks++;
    m_stats.numVertices = n1;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalChecks;

    emit isomorphismCheckCompleted(result, n1, timer.elapsed());
    return result;
}

/* ---- Automorphism generators ---- */

QVector<QVector<int>> GraphIsomorphism3::automorphismGenerators(const QVector<QVector<int>>& graph) const
{
    int n = graph.size();
    QVector<QVector<int>> generators;

    // Find all vertex permutations that preserve adjacency
    // Use refinement-based pruning for efficiency
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    // Check identity-like permutations by swapping pairs
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            QVector<int> testPerm = perm;
            std::swap(testPerm[i], testPerm[j]);

            bool valid = true;
            for (int a = 0; a < n && valid; ++a)
                for (int b = a + 1; b < n && valid; ++b)
                    if (graph[a][b] != graph[testPerm[a]][testPerm[b]])
                        valid = false;

            if (valid) generators.append(testPerm);
        }
    }
    return generators;
}

/* ---- Helpers ---- */

bool GraphIsomorphism3::coloringLess(const QVector<int>& a, const QVector<int>& b)
{
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
    }
    return a.size() < b.size();
}

QVector<int> GraphIsomorphism3::colorHistogram(const QVector<int>& coloring, int numColors)
{
    QVector<int> hist(numColors, 0);
    for (int c : coloring) { if (c < numColors) hist[c]++; }
    return hist;
}

bool GraphIsomorphism3::isDiscrete(const QVector<int>& coloring)
{
    QVector<int> seen(coloring.size() + 1, 0);
    for (int c : coloring) {
        if (c >= seen.size() || seen[c] > 0) return false;
        seen[c] = 1;
    }
    return true;
}

int GraphIsomorphism3::targetColorClass(const QVector<int>& coloring)
{
    QVector<int> counts(256, 0);
    for (int c : coloring) { if (c < 256) counts[c]++; }
    // Find smallest non-singleton color class
    int bestColor = -1, bestSize = 999999;
    for (int c = 0; c < 256; ++c) {
        if (counts[c] > 1 && counts[c] < bestSize) {
            bestSize = counts[c];
            bestColor = c;
        }
    }
    return bestColor;
}

/* ---- Reset ---- */

void GraphIsomorphism3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
