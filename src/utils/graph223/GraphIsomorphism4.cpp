/**
 * @file GraphIsomorphism4.cpp
 * @brief GraphIsomorphism4 实现
 *
 * 实现图同构：颜色细化(Weisfeiler-Leman)、个体化搜索、紧凑规范标号。
 */

#include "utils/graph223/GraphIsomorphism4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism4::GraphIsomorphism4(QObject *parent) : QObject(parent) {}
GraphIsomorphism4::~GraphIsomorphism4() = default;

/* ---- Neighborhood signature ---- */

QVector<QPair<int, QVector<int>>> GraphIsomorphism4::neighborhoodSignature(
    const QVector<QVector<int>>& graph,
    const QVector<int>& colors) const
{
    int n = graph.size();
    QVector<QPair<int, QVector<int>>> sigs(n);
    for (int v = 0; v < n; ++v) {
        QVector<int> neighborColors;
        for (int u = 0; u < n; ++u) {
            if (graph[v][u] != 0)
                neighborColors.append(colors[u]);
        }
        std::sort(neighborColors.begin(), neighborColors.end());
        sigs[v] = {colors[v], neighborColors};
    }
    return sigs;
}

/* ---- Compress colors to contiguous range ---- */

QVector<int> GraphIsomorphism4::compressColors(const QVector<int>& colors) const
{
    QVector<int> sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());

    QVector<int> result(colors.size());
    for (int i = 0; i < colors.size(); ++i) {
        result[i] = std::lower_bound(sorted.begin(), sorted.end(), colors[i])
                    - sorted.begin();
    }
    return result;
}

/* ---- Color refinement (1-WL) ---- */

QVector<int> GraphIsomorphism4::colorRefine(
    const QVector<QVector<int>>& graph, int maxIter) const
{
    int n = graph.size();
    if (n == 0) return {};

    // Initialize by degree
    QVector<int> colors(n, 0);
    for (int v = 0; v < n; ++v) {
        int deg = 0;
        for (int u = 0; u < n; ++u) deg += (graph[v][u] != 0) ? 1 : 0;
        colors[v] = deg;
    }
    colors = compressColors(colors);

    for (int iter = 0; iter < maxIter; ++iter) {
        auto sigs = neighborhoodSignature(graph, colors);

        // Assign new colors based on signatures
        QVector<QPair<int, QVector<int>>> uniqueSigs = sigs;
        std::sort(uniqueSigs.begin(), uniqueSigs.end());
        uniqueSigs.erase(std::unique(uniqueSigs.begin(), uniqueSigs.end()),
                         uniqueSigs.end());

        QVector<int> newColors(n);
        for (int v = 0; v < n; ++v) {
            newColors[v] = std::lower_bound(uniqueSigs.begin(),
                                             uniqueSigs.end(), sigs[v])
                           - uniqueSigs.begin();
        }

        if (newColors == colors) break;
        colors = newColors;
    }
    return colors;
}

/* ---- Is coloring discrete ---- */

bool GraphIsomorphism4::isDiscrete(const QVector<int>& colors)
{
    QVector<bool> seen(*std::max_element(colors.begin(), colors.end()) + 1, false);
    for (int c : colors) {
        if (seen[c]) return false;
        seen[c] = true;
    }
    return true;
}

/* ---- Count cells in partition ---- */

int GraphIsomorphism4::countCells(const QVector<int>& colors)
{
    QVector<int> sorted = colors;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    return sorted.size();
}

/* ---- Individualize a vertex ---- */

QVector<int> GraphIsomorphism4::individualize(
    const QVector<QVector<int>>& graph,
    const QVector<int>& colors, int vertex) const
{
    int n = colors.size();
    int maxColor = *std::max_element(colors.begin(), colors.end());

    // Find cell containing vertex
    int cellColor = colors[vertex];
    int newColor = maxColor + 1;

    QVector<int> newColors = colors;
    newColors[vertex] = newColor;

    // Re-refine after individualization
    return colorRefine(graph, 100);
}

/* ---- Build canonical from colors ---- */

QVector<int> GraphIsomorphism4::buildCanonicalFromColors(
    const QVector<QVector<int>>& graph,
    const QVector<int>& colors) const
{
    int n = graph.size();
    // Sort vertices by color, then encode adjacency rows
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&colors](int a, int b) {
        return colors[a] < colors[b];
    });

    QVector<int> canonical;
    canonical.reserve(n * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            canonical.append(graph[order[i]][order[j]]);
    return canonical;
}

/* ---- Check isomorphism ---- */

bool GraphIsomorphism4::isIsomorphic(const QVector<QVector<int>>& g1,
                                      const QVector<QVector<int>>& g2) const
{
    QElapsedTimer timer;
    timer.start();

    int n1 = g1.size(), n2 = g2.size();
    if (n1 != n2) return false;

    // Quick degree sequence check
    QVector<int> deg1(n1), deg2(n2);
    int e1 = 0, e2 = 0;
    for (int i = 0; i < n1; ++i)
        for (int j = 0; j < n1; ++j) {
            deg1[i] += (g1[i][j] != 0) ? 1 : 0;
            e1 += (g1[i][j] != 0) ? 1 : 0;
        }
    for (int i = 0; i < n2; ++i)
        for (int j = 0; j < n2; ++j) {
            deg2[i] += (g2[i][j] != 0) ? 1 : 0;
            e2 += (g2[i][j] != 0) ? 1 : 0;
        }

    std::sort(deg1.begin(), deg1.end());
    std::sort(deg2.begin(), deg2.end());
    if (deg1 != deg2 || e1 != e2) return false;

    // Color refinement
    QVector<int> c1 = colorRefine(g1);
    QVector<int> c2 = colorRefine(g2);

    // Compare sorted color histograms
    QVector<int> h1 = c1, h2 = c2;
    std::sort(h1.begin(), h1.end());
    std::sort(h2.begin(), h2.end());
    if (h1 != h2) return false;

    // If discrete coloring, compare canonical labels directly
    if (isDiscrete(c1)) {
        auto can1 = buildCanonicalFromColors(g1, c1);
        auto can2 = buildCanonicalFromColors(g2, c2);
        if (can1 != can2) return false;
    }

    // Individualization-refinement for non-discrete cases
    // Try individualizing the first non-trivial cell vertex
    for (int v = 0; v < n1; ++v) {
        QVector<int> ic1 = individualize(g1, c1, v);
        QVector<int> ic2 = individualize(g2, c2, v);

        QVector<int> sh1 = ic1, sh2 = ic2;
        std::sort(sh1.begin(), sh1.end());
        std::sort(sh2.begin(), sh2.end());
        if (sh1 != sh2) return false;
    }

    auto self = const_cast<GraphIsomorphism4*>(this);
    self->m_stats.totalOps++;
    self->m_stats.numVertices = n1;
    self->m_stats.numEdges = e1 / 2;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit isomorphismChecked(true, n1, timer.elapsed());

    return true;
}

/* ---- Canonical label ---- */

QVector<int> GraphIsomorphism4::canonicalLabel(
    const QVector<QVector<int>>& graph) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> colors = colorRefine(graph);

    // If not discrete, individualize recursively to find best
    if (!isDiscrete(colors)) {
        int n = graph.size();
        for (int v = 0; v < n && !isDiscrete(colors); ++v)
            colors = individualize(graph, colors, v);
    }

    auto result = buildCanonicalFromColors(graph, colors);

    auto self = const_cast<GraphIsomorphism4*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Find automorphisms ---- */

QVector<QVector<int>> GraphIsomorphism4::findAutomorphisms(
    const QVector<QVector<int>>& graph) const
{
    int n = graph.size();
    QVector<QVector<int>> autos;

    QVector<int> idPerm(n);
    for (int i = 0; i < n; ++i) idPerm[i] = i;

    // Generate candidate permutations from color classes
    QVector<int> colors = colorRefine(graph);

    // Group vertices by color
    QVector<QVector<int>> colorClasses(*std::max_element(colors.begin(), colors.end()) + 1);
    for (int v = 0; v < n; ++v)
        colorClasses[colors[v]].append(v);

    // Permute within same color class to find automorphisms
    for (const auto& cls : colorClasses) {
        if (cls.size() <= 1) continue;
        // Try swapping pairs within the class
        for (int i = 0; i < cls.size(); ++i) {
            for (int j = i + 1; j < cls.size(); ++j) {
                QVector<int> perm = idPerm;
                std::swap(perm[cls[i]], perm[cls[j]]);
                // Verify: apply permutation to graph
                bool valid = true;
                for (int a = 0; a < n && valid; ++a)
                    for (int b = 0; b < n && valid; ++b)
                        if (graph[a][b] != graph[perm[a]][perm[b]])
                            valid = false;
                if (valid) autos.append(perm);
            }
        }
    }
    return autos;
}

/* ---- Reset ---- */

void GraphIsomorphism4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
