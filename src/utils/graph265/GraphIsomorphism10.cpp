/**
 * @file GraphIsomorphism10.cpp
 * @brief GraphIsomorphism10 实现
 *
 * 实现图同构：个体化-细化与自同构群检测搜索树剪枝。
 */

#include "utils/graph265/GraphIsomorphism10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism10::GraphIsomorphism10(QObject *parent) : QObject(parent) {}
GraphIsomorphism10::~GraphIsomorphism10() = default;

/* ---- Count edges ---- */

int GraphIsomorphism10::countEdges(const QVector<QVector<int>>& adj)
{
    int n = adj.size();
    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            edges += adj[i][j];
    return edges;
}

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism10::degreeSequence(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            deg[i] += adj[i][j];
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

/* ---- Weisfeiler-Lehman color refinement ---- */

QVector<int> GraphIsomorphism10::refine(const QVector<QVector<int>>& adj,
                                           const QVector<int>& coloring) const
{
    int n = adj.size();
    QVector<int> newColor = coloring;

    // Iterative refinement until stable
    for (int iter = 0; iter < n; ++iter) {
        QVector<QPair<int, QVector<int>>> signatures(n);
        for (int v = 0; v < n; ++v) {
            signatures[v].first = newColor[v];
            QVector<int> neighborColors;
            for (int u = 0; u < n; ++u)
                if (adj[v][u]) neighborColors.append(newColor[u]);
            std::sort(neighborColors.begin(), neighborColors.end());
            signatures[v].second = neighborColors;
        }

        // Assign new colors based on signatures
        QVector<int> sorted = newColor;
        std::sort(sorted.begin(), sorted.end());
        sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());

        QVector<int> nextColor(n);
        int nextId = 0;
        for (int v = 0; v < n; ++v) {
            int found = -1;
            for (int c = 0; c < n; ++c) {
                if (c != v && signatures[c].first == signatures[v].first &&
                    signatures[c].second == signatures[v].second) {
                    found = nextColor[c]; break;
                }
            }
            if (found >= 0) nextColor[v] = found;
            else nextColor[v] = nextId++;
        }

        if (nextColor == newColor) break;
        newColor = nextColor;
    }
    return newColor;
}

/* ---- Individualize vertex ---- */

QVector<int> GraphIsomorphism10::individualize(const QVector<int>& coloring,
                                                  int v, int newColor) const
{
    QVector<int> result = coloring;
    result[v] = newColor;
    return result;
}

/* ---- Check discrete coloring ---- */

bool GraphIsomorphism10::isDiscrete(const QVector<int>& coloring) const
{
    int n = coloring.size();
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (coloring[i] == coloring[j]) return false;
    return true;
}

/* ---- Check automorphism ---- */

bool GraphIsomorphism10::isAutomorphism(const QVector<int>& perm,
                                           const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (adj[i][j] != adj[perm[i]][perm[j]]) return false;
    return true;
}

/* ---- Select target vertex ---- */

int GraphIsomorphism10::selectTargetVertex(const QVector<int>& coloring) const
{
    // Find smallest non-singleton color class, pick first vertex
    int n = coloring.size();
    for (int color = 0; color < n; ++color) {
        int count = 0;
        int first = -1;
        for (int v = 0; v < n; ++v) {
            if (coloring[v] == color) { count++; if (first < 0) first = v; }
        }
        if (count > 1) return first;
    }
    return -1;
}

/* ---- Search tree ---- */

void GraphIsomorphism10::searchTree(const SearchNode& node)
{
    if (m_found) return;
    m_stats.searchNodesExplored++;

    QVector<int> refined = refine(m_adj1, node.coloring);

    // Check if this could be a valid mapping
    bool valid = true;
    int n = m_n;
    for (int v = 0; v < n && valid; ++v) {
        // Verify color structure matches graph2
        for (int u = 0; u < n && valid; ++u) {
            if (m_adj1[v][u]) {
                // Check if there exists consistent mapping to adj2
                bool found = false;
                for (int v2 = 0; v2 < n; ++v2) {
                    for (int u2 = 0; u2 < n; ++u2) {
                        if (refined[v] == refined[v2] && refined[u] == refined[u2] && m_adj2[v2][u2])
                            found = true;
                    }
                }
                if (!found) valid = false;
            }
        }
    }

    if (!valid) { m_stats.pruningCuts++; return; }

    if (isDiscrete(refined)) {
        // Extract permutation
        QVector<int> perm(n);
        for (int v = 0; v < n; ++v) perm[v] = refined[v];
        if (isAutomorphism(perm, m_adj1)) {
            m_automorphisms.append(perm);
            m_stats.automorphismsFound++;
        }
        // Check if this is a valid isomorphism
        bool iso = true;
        for (int i = 0; i < n && iso; ++i)
            for (int j = 0; j < n && iso; ++j)
                if (m_adj1[i][j] != m_adj2[perm[i] < n ? perm[i] : i]
                    [perm[j] < n ? perm[j] : j]) iso = false;
        if (iso) { m_mapping = perm; m_found = true; }
        return;
    }

    // Branch: individualize target vertex
    int target = selectTargetVertex(refined);
    if (target < 0) return;

    int newColor = n + node.depth + 1;
    SearchNode child;
    child.coloring = individualize(refined, target, newColor);
    child.depth = node.depth + 1;
    child.targetVertex = target;
    searchTree(child);
}

/* ---- Isomorphic check ---- */

bool GraphIsomorphism10::isIsomorphic(const QVector<QVector<int>>& adj1,
                                        const QVector<QVector<int>>& adj2)
{
    QElapsedTimer timer;
    timer.start();

    m_n = adj1.size();
    if (m_n != adj2.size()) return false;
    if (m_n == 0) return true;

    m_adj1 = adj1;
    m_adj2 = adj2;
    m_found = false;
    m_mapping.clear();
    m_automorphisms.clear();

    // Quick checks: edge count and degree sequence
    if (countEdges(adj1) != countEdges(adj2)) return false;
    if (degreeSequence(adj1) != degreeSequence(adj2)) return false;

    // Initial coloring: degree-based
    QVector<int> initColor(m_n);
    for (int v = 0; v < m_n; ++v) {
        int deg = 0;
        for (int u = 0; u < m_n; ++u) deg += adj1[v][u];
        initColor[v] = deg;
    }

    SearchNode root;
    root.coloring = initColor;
    root.depth = 0;
    searchTree(root);

    m_stats.numVertices = m_n;
    m_stats.numEdges = countEdges(adj1);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit searchCompleted(m_stats.searchNodesExplored, m_stats.pruningCuts, timer.elapsed());
    return m_found;
}

/* ---- Find automorphisms ---- */

QVector<QVector<int>> GraphIsomorphism10::findAutomorphisms(const QVector<QVector<int>>& adj)
{
    // A graph is isomorphic to itself, collect all automorphisms
    isIsomorphic(adj, adj);
    return m_automorphisms;
}

QVector<int> GraphIsomorphism10::mapping() const { return m_mapping; }

/* ---- Reset ---- */

void GraphIsomorphism10::resetStatistics()
{
    m_mapping.clear();
    m_automorphisms.clear();
    m_found = false;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
