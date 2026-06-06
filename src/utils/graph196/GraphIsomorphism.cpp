/**
 * @file GraphIsomorphism.cpp
 * @brief GraphIsomorphism 实现
 *
 * 实现图同构检测：度序列过滤、Weisfeiler-Lehman迭代分区精炼、规范标记、同构判定。
 */

#include "utils/graph196/GraphIsomorphism.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism::GraphIsomorphism(QObject *parent) : QObject(parent) {}
GraphIsomorphism::~GraphIsomorphism() = default;

/* ---- Degree computation ---- */

QVector<int> GraphIsomorphism::computeDegrees(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        deg[i] = adj[i].size();
    return deg;
}

QVector<int> GraphIsomorphism::degreeSequence(const QVector<QVector<int>>& adj) const
{
    auto deg = computeDegrees(adj);
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

int GraphIsomorphism::countEdges(const QVector<QVector<int>>& adj) const
{
    int edges = 0;
    for (const auto& row : adj) edges += row.size();
    return edges / 2; // Undirected: each edge counted twice
}

/* ---- Weisfeiler-Lehman refinement ---- */

QVector<int> GraphIsomorphism::wlRefinement(const QVector<QVector<int>>& adj, int iterations) const
{
    int n = adj.size();
    if (n == 0) return {};

    // Initialize coloring by degree
    QVector<int> coloring = computeDegrees(adj);

    for (int iter = 0; iter < iterations; ++iter) {
        // Build multiset labels: (own_color, sorted(neighbor_colors))
        QVector<QPair<int, QVector<int>>> labels(n);
        for (int v = 0; v < n; ++v) {
            QVector<int> neighborColors;
            neighborColors.reserve(adj[v].size());
            for (int u : adj[v])
                neighborColors.append(coloring[u]);
            std::sort(neighborColors.begin(), neighborColors.end());
            labels[v] = qMakePair(coloring[v], neighborColors);
        }

        // Relabel: compress unique labels to integers
        QVector<QPair<int, QVector<int>>> uniqueLabels = labels;
        std::sort(uniqueLabels.begin(), uniqueLabels.end());
        uniqueLabels.erase(std::unique(uniqueLabels.begin(), uniqueLabels.end()),
                           uniqueLabels.end());

        for (int v = 0; v < n; ++v) {
            int idx = std::lower_bound(uniqueLabels.begin(), uniqueLabels.end(), labels[v])
                      - uniqueLabels.begin();
            coloring[v] = idx;
        }
    }

    return coloring;
}

/* ---- Serialize adjacency ---- */

QString GraphIsomorphism::serializeAdj(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QString result;
    for (int i = 0; i < n; ++i) {
        QVector<int> sorted = adj[i];
        std::sort(sorted.begin(), sorted.end());
        for (int j : sorted)
            result += QString("%1-%2|").arg(i).arg(j);
    }
    return result;
}

/* ---- Adjacency equality ---- */

bool GraphIsomorphism::adjEqual(const QVector<QVector<int>>& a,
                                  const QVector<QVector<int>>& b) const
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); ++i) {
        QVector<int> sa = a[i], sb = b[i];
        std::sort(sa.begin(), sa.end());
        std::sort(sb.begin(), sb.end());
        if (sa != sb) return false;
    }
    return true;
}

/* ---- Generate permutation candidates ---- */

QVector<QVector<int>> GraphIsomorphism::generateCandidates(const QVector<QVector<int>>& adj,
                                                              const QVector<int>& coloring) const
{
    int n = adj.size();
    if (n == 0) return {};

    // Group vertices by color
    QMap<int, QVector<int>> colorGroups;
    for (int i = 0; i < n; ++i)
        colorGroups[coloring[i]].append(i);

    // Generate permutations: one per color group permutation
    // For efficiency, just try identity + a few swaps within same-color groups
    QVector<QVector<int>> candidates;

    // Identity permutation
    QVector<int> identity(n);
    for (int i = 0; i < n; ++i) identity[i] = i;
    candidates.append(identity);

    // Swap within each color group
    for (auto it = colorGroups.begin(); it != colorGroups.end(); ++it) {
        const QVector<int>& group = it.value();
        if (group.size() >= 2) {
            for (int i = 0; i < qMin(group.size(), 3); ++i) {
                for (int j = i + 1; j < qMin(group.size(), 4); ++j) {
                    QVector<int> perm = identity;
                    std::swap(perm[group[i]], perm[group[j]]);
                    candidates.append(perm);
                }
            }
        }
    }

    return candidates;
}

/* ---- Permute graph ---- */

QVector<QVector<int>> GraphIsomorphism::permuteGraph(const QVector<QVector<int>>& adj,
                                                        const QVector<int>& perm) const
{
    int n = adj.size();
    QVector<QVector<int>> result(n);
    for (int i = 0; i < n; ++i) {
        int pi = perm[i];
        for (int j : adj[i])
            result[pi].append(perm[j]);
    }
    return result;
}

/* ---- Canonical label ---- */

QString GraphIsomorphism::canonicalLabel(const QVector<QVector<int>>& adj) const
{
    auto coloring = wlRefinement(adj, 10);
    auto candidates = generateCandidates(adj, coloring);

    QString bestLabel;
    for (const auto& perm : candidates) {
        auto permuted = permuteGraph(adj, perm);
        // Sort adjacency for canonical form
        for (auto& row : permuted)
            std::sort(row.begin(), row.end());
        QString label = serializeAdj(permuted);
        if (bestLabel.isEmpty() || label < bestLabel)
            bestLabel = label;
    }
    return bestLabel;
}

/* ---- Main isomorphism check ---- */

bool GraphIsomorphism::isIsomorphic(const QVector<QVector<int>>& adj1,
                                      const QVector<QVector<int>>& adj2)
{
    QElapsedTimer timer;
    timer.start();

    int n1 = adj1.size(), n2 = adj2.size();

    // Quick filter: vertex count
    if (n1 != n2) return false;

    // Quick filter: edge count
    if (countEdges(adj1) != countEdges(adj2)) return false;

    // Quick filter: degree sequence
    auto deg1 = degreeSequence(adj1);
    auto deg2 = degreeSequence(adj2);
    if (deg1 != deg2) {
        m_stats.filteredByDegree++;
        m_stats.totalComparisons++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        emit comparisonCompleted(false, n1);
        return false;
    }

    // Weisfeiler-Lehman hash comparison
    auto wl1 = wlRefinement(adj1, 10);
    auto wl2 = wlRefinement(adj2, 10);

    // Sort colorings for comparison
    auto sw1 = wl1, sw2 = wl2;
    std::sort(sw1.begin(), sw1.end());
    std::sort(sw2.begin(), sw2.end());
    if (sw1 != sw2) {
        m_stats.totalComparisons++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;
        emit comparisonCompleted(false, n1);
        return false;
    }

    // Canonical label comparison for final verification
    QString can1 = canonicalLabel(adj1);
    QString can2 = canonicalLabel(adj2);
    bool result = (can1 == can2);

    if (result) m_stats.isomorphicCount++;
    m_stats.totalComparisons++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComparisons;

    emit comparisonCompleted(result, n1);
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
