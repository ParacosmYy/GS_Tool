/**
 * @file GraphIsomorphism13.cpp
 * @brief GraphIsomorphism13 实现
 *
 * 实现图同构：个体化-细化与搜索树剪枝认证非同构测试。
 */

#include "utils/graph289/GraphIsomorphism13.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism13::GraphIsomorphism13(QObject *parent)
    : QObject(parent) {}

GraphIsomorphism13::~GraphIsomorphism13() = default;

/* ---- Compute degree sequence for quick rejection ---- */

QVector<int> GraphIsomorphism13::degreeSequence(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> degrees(n);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j)
            if (adj[i][j] != 0) deg++;
        degrees[i] = deg;
    }
    std::sort(degrees.begin(), degrees.end());
    return degrees;
}

/* ---- Refine coloring using neighbor color multisets ---- */

bool GraphIsomorphism13::refine(const QVector<QVector<int>>& adj,
                                 QVector<int>& coloring) const
{
    int n = adj.size();
    bool changed = true;
    int maxColor = 0;
    for (int c : coloring) maxColor = qMax(maxColor, c);

    while (changed) {
        changed = false;
        // For each vertex, compute signature = (own color, sorted neighbor colors)
        QVector<QPair<int, QVector<int>>> signatures(n);
        for (int v = 0; v < n; ++v) {
            signatures[v].first = coloring[v];
            QVector<int> neighborColors;
            for (int u = 0; u < n; ++u) {
                if (adj[v][u] != 0) neighborColors.append(coloring[u]);
            }
            std::sort(neighborColors.begin(), neighborColors.end());
            signatures[v].second = neighborColors;
        }

        // Assign new colors based on unique signatures
        QMap<QPair<int, QVector<int>>, int> sigToColor;
        QVector<int> newColoring(n);
        int nextColor = 0;
        for (int v = 0; v < n; ++v) {
            auto sig = qMakePair(signatures[v].first, signatures[v].second);
            if (!sigToColor.contains(sig))
                sigToColor[sig] = nextColor++;
            newColoring[v] = sigToColor[sig];
        }

        if (newColoring != coloring) {
            coloring = newColoring;
            changed = true;
        }

        // Check if all cells are discrete (1 element each)
        QMap<int, int> colorCount;
        for (int c : coloring) colorCount[c]++;
        bool discrete = true;
        for (auto it = colorCount.begin(); it != colorCount.end(); ++it)
            if (it.value() > 1) { discrete = false; break; }
        if (discrete) break;
    }
    return true;
}

/* ---- Get color class sizes ---- */

QVector<int> GraphIsomorphism13::colorClassSizes(const QVector<int>& coloring, int n) const
{
    QMap<int, int> counts;
    for (int i = 0; i < n; ++i) counts[coloring[i]]++;
    QVector<int> sizes;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        sizes.append(it.value());
    std::sort(sizes.begin(), sizes.end());
    return sizes;
}

/* ---- Individualize a vertex ---- */

QVector<int> GraphIsomorphism13::individualize(const QVector<int>& coloring,
                                                int vertex, int n) const
{
    // Find max color
    int maxC = 0;
    for (int c : coloring) maxC = qMax(maxC, c);

    QVector<int> newColoring = coloring;
    newColoring[vertex] = maxC + 1;
    return newColoring;
}

/* ---- Check color compatibility ---- */

bool GraphIsomorphism13::colorCompatible(const QVector<int>& c1,
                                          const QVector<int>& c2, int n) const
{
    return colorClassSizes(c1, n) == colorClassSizes(c2, n);
}

/* ---- DFS search tree with pruning ---- */

bool GraphIsomorphism13::searchTree(const QVector<QVector<int>>& adj1,
                                     const QVector<QVector<int>>& adj2,
                                     const SearchNode& node1, const SearchNode& node2,
                                     QVector<int>& mapping, int& explored)
{
    explored++;
    if (explored > 100000) return false;  // Safety limit

    int n = adj1.size();

    // Check if mapping is complete (all discrete)
    bool complete = true;
    for (int i = 0; i < n; ++i) {
        int count = 0;
        for (int j = 0; j < n; ++j)
            if (node1.coloring[j] == node1.coloring[i]) count++;
        if (count > 1) { complete = false; break; }
    }

    if (complete) {
        // Extract mapping from discrete coloring
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (node1.coloring[i] == node2.coloring[j]) {
                    mapping[i] = j;
                    break;
                }
            }
        }
        // Verify adjacency preservation
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if ((adj1[i][j] != 0) != (adj2[mapping[i]][mapping[j]] != 0))
                    return false;
            }
        }
        return true;
    }

    // Find the first non-singleton color cell to individualize
    int targetColor = -1;
    for (int c = 0; c < n; ++c) {
        int count = 0;
        for (int i = 0; i < n; ++i)
            if (node1.coloring[i] == c) count++;
        if (count > 1) { targetColor = c; break; }
    }
    if (targetColor < 0) return false;

    // Try individualizing each vertex in the target cell of graph 1
    for (int v = 0; v < n; ++v) {
        if (node1.coloring[v] != targetColor) continue;

        // Try matching v to each vertex in the same color cell of graph 2
        for (int u = 0; u < n; ++u) {
            if (node2.coloring[u] != targetColor) continue;

            SearchNode child1, child2;
            child1.coloring = individualize(node1.coloring, v, n);
            child2.coloring = individualize(node2.coloring, u, n);
            child1.individualized = v;
            child2.individualized = u;
            child1.depth = node1.depth + 1;
            child2.depth = node2.depth + 1;

            // Refine both
            refine(adj1, child1.coloring);
            refine(adj2, child2.coloring);

            // Prune: check color compatibility
            if (!colorCompatible(child1.coloring, child2.coloring, n))
                continue;

            if (searchTree(adj1, adj2, child1, child2, mapping, explored))
                return true;
        }
        // Only need to try one vertex from graph1's cell (symmetry breaking)
        break;
    }

    return false;
}

/* ---- Main isomorphism test ---- */

bool GraphIsomorphism13::isIsomorphic(const QVector<QVector<int>>& adj1,
                                       const QVector<QVector<int>>& adj2)
{
    QElapsedTimer timer;
    timer.start();

    int n = adj1.size();
    if (n != static_cast<int>(adj2.size())) return false;
    if (n == 0) return true;

    // Quick rejection: degree sequence
    if (degreeSequence(adj1) != degreeSequence(adj2)) return false;

    // Initial coloring: by degree
    QVector<int> color1(n, 0), color2(n, 0);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j) if (adj1[i][j] != 0) deg++;
        color1[i] = deg;
    }
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j) if (adj2[i][j] != 0) deg++;
        color2[i] = deg;
    }

    // Refine initial coloring
    refine(adj1, color1);
    refine(adj2, color2);

    // Quick check: color class sizes must match
    if (!colorCompatible(color1, color2, n)) {
        double elapsed = timer.elapsed();
        m_stats.totalOps++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit testDone(false, 0, elapsed);
        return false;
    }

    // Search tree
    SearchNode root1, root2;
    root1.coloring = color1;
    root2.coloring = color2;
    root1.depth = 0;
    root2.depth = 0;

    QVector<int> mapping(n, -1);
    int explored = 0;
    bool result = searchTree(adj1, adj2, root1, root2, mapping, explored);

    double elapsed = timer.elapsed();
    m_stats.numVertices = n;
    m_stats.searchNodesExplored = explored;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit testDone(result, explored, elapsed);

    return result;
}

/* ---- Find mapping ---- */

QVector<int> GraphIsomorphism13::findMapping(const QVector<QVector<int>>& adj1,
                                              const QVector<QVector<int>>& adj2)
{
    int n = adj1.size();
    if (n != static_cast<int>(adj2.size())) return {};

    QVector<int> mapping(n, -1);
    int explored = 0;

    // Initial coloring by degree
    QVector<int> color1(n), color2(n);
    for (int i = 0; i < n; ++i) {
        int d = 0;
        for (int j = 0; j < n; ++j) if (adj1[i][j]) d++;
        color1[i] = d;
    }
    for (int i = 0; i < n; ++i) {
        int d = 0;
        for (int j = 0; j < n; ++j) if (adj2[i][j]) d++;
        color2[i] = d;
    }
    refine(adj1, color1);
    refine(adj2, color2);

    SearchNode r1, r2;
    r1.coloring = color1; r2.coloring = color2;
    r1.depth = 0; r2.depth = 0;

    if (searchTree(adj1, adj2, r1, r2, mapping, explored))
        return mapping;
    return {};
}

/* ---- Canonical label ---- */

QVector<int> GraphIsomorphism13::canonicalLabel(const QVector<QVector<int>>& adj)
{
    int n = adj.size();
    if (n == 0) return {};

    // Use degree-based initial coloring
    QVector<int> coloring(n);
    for (int i = 0; i < n; ++i) {
        int d = 0;
        for (int j = 0; j < n; ++j) if (adj[i][j]) d++;
        coloring[i] = d;
    }
    refine(adj, coloring);

    // Build canonical label from refined coloring + adjacency
    // Sort vertices by (color, neighbor pattern)
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        if (coloring[a] != coloring[b]) return coloring[a] < coloring[b];
        for (int k = 0; k < n; ++k) {
            if (adj[a][k] != adj[b][k]) return adj[a][k] < adj[b][k];
        }
        return a < b;
    });

    // Relabel adjacency matrix in canonical order
    QVector<int> label(n * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            label[i * n + j] = adj[order[i]][order[j]];

    return label;
}

/* ---- Reset ---- */

void GraphIsomorphism13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
