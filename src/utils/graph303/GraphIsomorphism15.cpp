/**
 * @file GraphIsomorphism15.cpp
 * @brief GraphIsomorphism15 实现
 *
 * 实现图同构：个体化-细化与广度优先搜索树认证的规范标号算法。
 */

#include "utils/graph303/GraphIsomorphism15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism15::GraphIsomorphism15(QObject *parent)
    : QObject(parent) {}

GraphIsomorphism15::~GraphIsomorphism15() = default;

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism15::degreeSequence(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            deg[i] += adj[i][j];
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

/* ---- Initial coloring by degree ---- */

QVector<int> GraphIsomorphism15::initialColoring(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> colors(n);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j) deg += adj[i][j];
        colors[i] = deg;
    }
    return colors;
}

/* ---- Refine coloring using neighbor color multisets ----
 *
 * Weisfeiler-Lehman style refinement: each vertex gets a new color
 * based on the sorted multiset of its neighbors' colors.
 */
QVector<int> GraphIsomorphism15::refineColoring(const QVector<QVector<int>>& adj,
                                                  const QVector<int>& colors) const
{
    int n = adj.size();
    QVector<QVector<int>> neighborColors(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adj[i][j] != 0) {
                neighborColors[i].append(colors[j]);
            }
        }
        std::sort(neighborColors[i].begin(), neighborColors[i].end());
    }

    // Assign new colors based on (own color, sorted neighbor colors) pair
    QVector<QPair<int, QVector<int>>> signatures(n);
    for (int i = 0; i < n; ++i) {
        signatures[i] = {colors[i], neighborColors[i]};
    }

    // Map unique signatures to integer colors
    QVector<int> newColors(n);
    int nextColor = 0;
    QVector<QPair<int, QVector<int>>> seen;
    for (int i = 0; i < n; ++i) {
        int found = -1;
        for (int s = 0; s < seen.size(); ++s) {
            if (seen[s].first == signatures[i].first &&
                seen[s].second == signatures[i].second) {
                found = s;
                break;
            }
        }
        if (found >= 0) {
            newColors[i] = found;
        } else {
            newColors[i] = nextColor;
            seen.append(signatures[i]);
            nextColor++;
        }
    }
    return newColors;
}

/* ---- Individualization: assign a unique color to a vertex ---- */

QVector<int> GraphIsomorphism15::individualize(const QVector<int>& colors, int vertex) const
{
    int n = colors.size();
    int maxColor = *std::max_element(colors.begin(), colors.end());
    QVector<int> result = colors;
    result[vertex] = maxColor + 1;
    return result;
}

/* ---- BFS tree certification from a root ---- */

QVector<int> GraphIsomorphism15::bfsCertificate(const QVector<QVector<int>>& adj,
                                                  int root, const QVector<int>& colors) const
{
    int n = adj.size();
    QVector<int> cert;
    QVector<bool> visited(n, false);
    QVector<int> queue;
    queue.append(root);
    visited[root] = true;

    while (!queue.isEmpty()) {
        int cur = queue.takeFirst();
        cert.append(colors[cur]);

        // Collect neighbor info: sorted by (color, visited) for determinism
        QVector<QPair<int, bool>> neighbors;
        for (int j = 0; j < n; ++j) {
            if (adj[cur][j] != 0) {
                neighbors.append({colors[j], visited[j]});
            }
        }
        std::sort(neighbors.begin(), neighbors.end());

        for (const auto& nb : neighbors) {
            cert.append(nb.first); // neighbor color
            cert.append(nb.second ? 1 : 0); // visited flag

            // Find the actual neighbor index to enqueue
            for (int j = 0; j < n; ++j) {
                if (adj[cur][j] != 0 && colors[j] == nb.first && !visited[j]) {
                    visited[j] = true;
                    queue.append(j);
                    break;
                }
            }
        }
        cert.append(-1); // Level separator
    }
    return cert;
}

/* ---- Integer hash of adjacency under permutation ---- */

QVector<int> GraphIsomorphism15::permutedAdjHash(const QVector<QVector<int>>& adj,
                                                   const QVector<int>& perm) const
{
    int n = adj.size();
    QVector<int> hash;
    hash.reserve(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            hash.append(adj[perm[i]][perm[j]]);
        }
    }
    return hash;
}

/* ---- Canonical labeling ---- */

GraphIsomorphism15::CanonResult GraphIsomorphism15::canonicalLabel(
    const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    CanonResult result;
    if (n == 0) return result;

    QVector<int> colors = initialColoring(adj);

    // Iterative refinement until stable
    for (int iter = 0; iter < n; ++iter) {
        QVector<int> newColors = refineColoring(adj, colors);
        if (newColors == colors) break;
        colors = newColors;
    }

    // Try individualization from each vertex, pick best BFS certificate
    QVector<int> bestPerm;
    QVector<int> bestHash;

    for (int root = 0; root < n; ++root) {
        QVector<int> indColors = individualize(colors, root);

        // Refine again after individualization
        for (int iter = 0; iter < n; ++iter) {
            QVector<int> newColors = refineColoring(adj, indColors);
            if (newColors == indColors) break;
            indColors = newColors;
        }

        // Build permutation from color ordering
        QVector<int> perm(n);
        QVector<QPair<int, int>> colorIdx(n);
        for (int i = 0; i < n; ++i)
            colorIdx[i] = {indColors[i], i};
        std::sort(colorIdx.begin(), colorIdx.end());
        for (int i = 0; i < n; ++i)
            perm[colorIdx[i].second] = i;

        QVector<int> hash = permutedAdjHash(adj, perm);

        if (bestHash.isEmpty() || hash < bestHash) {
            bestHash = hash;
            bestPerm = perm;
        }
    }

    result.permutation = bestPerm;
    result.certHash = bestHash;
    return result;
}

/* ---- Isomorphism test ---- */

GraphIsomorphism15::IsoResult GraphIsomorphism15::isIsomorphic(
    const QVector<QVector<int>>& g1, const QVector<QVector<int>>& g2) const
{
    QElapsedTimer timer;
    timer.start();

    IsoResult result;
    int n1 = g1.size();
    int n2 = g2.size();

    // Quick checks
    if (n1 != n2) return result;

    int n = n1;
    if (n == 0) {
        result.isomorphic = true;
        return result;
    }

    // Compare degree sequences
    QVector<int> deg1 = degreeSequence(g1);
    QVector<int> deg2 = degreeSequence(g2);
    if (deg1 != deg2) return result;

    // Compare edge counts
    int edges1 = 0, edges2 = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            edges1 += g1[i][j];
            edges2 += g2[i][j];
        }
    if (edges1 != edges2) return result;

    // Compute canonical labels and compare
    CanonResult c1 = canonicalLabel(g1);
    CanonResult c2 = canonicalLabel(g2);

    if (c1.certHash == c2.certHash) {
        result.isomorphic = true;
        // Build mapping from g1 to g2
        result.mapping.resize(n);
        for (int i = 0; i < n; ++i)
            result.mapping[c1.permutation[i]] = c2.permutation[i];
    }

    result.timeMs = timer.elapsed();
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
