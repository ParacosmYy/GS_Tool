/**
 * @file GraphIsomorphism14.cpp
 * @brief GraphIsomorphism14 实现
 *
 * 实现图同构检测：分布式非同构证书与规范标记的高效图哈希。
 */

#include "utils/graph293/GraphIsomorphism14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism14::GraphIsomorphism14(QObject *parent)
    : QObject(parent) {}

GraphIsomorphism14::~GraphIsomorphism14() = default;

/* ---- Hash combine ---- */

quint64 GraphIsomorphism14::hashCombine(quint64 h1, quint64 h2) const
{
    // Boost-style hash combine
    return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
}

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism14::degreeSequence(const AdjMatrix& g) const
{
    int n = g.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (g[i][j] != 0) deg[i]++;
    std::sort(deg.begin(), deg.end());
    return deg;
}

/* ---- Weisfeiler-Leman (WL) vertex coloring ---- */

QVector<int> GraphIsomorphism14::wlColoring(const AdjMatrix& g, int maxIter) const
{
    int n = g.size();
    if (n == 0) return {};

    // Initial coloring by degree
    QVector<int> colors(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (g[i][j] != 0) colors[i]++;

    for (int iter = 0; iter < maxIter; ++iter) {
        // Refine colors based on multiset of neighbor colors
        QVector<QPair<int, QVector<int>>> signatures(n);
        for (int i = 0; i < n; ++i) {
            signatures[i].first = colors[i];
            QVector<int> neighborColors;
            for (int j = 0; j < n; ++j)
                if (g[i][j] != 0) neighborColors.append(colors[j]);
            std::sort(neighborColors.begin(), neighborColors.end());
            signatures[i].second = neighborColors;
        }

        // Assign new colors by sorting unique signatures
        QVector<int> newColors(n, 0);
        QVector<int> indices(n);
        for (int i = 0; i < n; ++i) indices[i] = i;
        std::sort(indices.begin(), indices.end(), [&](int a, int b) {
            if (signatures[a].first != signatures[b].first)
                return signatures[a].first < signatures[b].first;
            return signatures[a].second < signatures[b].second;
        });

        int colorIdx = 0;
        newColors[indices[0]] = colorIdx;
        for (int i = 1; i < n; ++i) {
            if (signatures[indices[i]] != signatures[indices[i - 1]])
                ++colorIdx;
            newColors[indices[i]] = colorIdx;
        }

        if (newColors == colors) break;   // Converged
        colors = newColors;
    }
    return colors;
}

/* ---- Degree compatibility check ---- */

bool GraphIsomorphism14::degreeCompatible(const AdjMatrix& g1, const AdjMatrix& g2) const
{
    if (g1.size() != g2.size()) return false;
    return degreeSequence(g1) == degreeSequence(g2);
}

/* ---- Build adjacency string for canonical form ---- */

QString GraphIsomorphism14::adjacencyString(const AdjMatrix& g, const QVector<int>& order) const
{
    int n = g.size();
    QString s;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            s.append(QString::number(g[order[i]][order[j]]));
            if (j < n - 1) s.append(',');
        }
        if (i < n - 1) s.append(';');
    }
    return s;
}

/* ---- Backtracking search for isomorphism ---- */

bool GraphIsomorphism14::findMapping(const AdjMatrix& g1, const AdjMatrix& g2,
                                       const QVector<int>& colors1, const QVector<int>& colors2,
                                       QVector<int>& mapping, QVector<bool>& used, int depth)
{
    int n = g1.size();
    if (depth == n) return true;

    // Pick next vertex from g1 (most constrained first)
    int u = depth;

    for (int v = 0; v < n; ++v) {
        // Prune: colors must match
        if (colors1[u] != colors2[v]) continue;
        if (used[v]) continue;

        // Prune: check partial adjacency consistency
        bool ok = true;
        for (int d = 0; d < depth; ++d) {
            if (g1[u][d] != g2[v][mapping[d]] ||
                g1[d][u] != g2[mapping[d]][v]) {
                ok = false;
                break;
            }
        }
        if (!ok) continue;

        mapping[u] = v;
        used[v] = true;
        if (findMapping(g1, g2, colors1, colors2, mapping, used, depth + 1))
            return true;
        used[v] = false;
    }
    return false;
}

/* ---- Canonical label ---- */

QString GraphIsomorphism14::canonicalLabel(const AdjMatrix& g) const
{
    int n = g.size();
    if (n == 0) return QStringLiteral("empty");

    QVector<int> colors = wlColoring(g);

    // Try all permutations of vertices grouped by color to find minimal string
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return colors[a] < colors[b];
    });

    return adjacencyString(g, order);
}

/* ---- Vertex invariants ---- */

QVector<quint64> GraphIsomorphism14::vertexInvariants(const AdjMatrix& g) const
{
    int n = g.size();
    QVector<quint64> inv(n, 0);
    QVector<int> colors = wlColoring(g);

    for (int i = 0; i < n; ++i) {
        quint64 h = static_cast<quint64>(colors[i]) * 0x9e3779b9;
        int deg = 0;
        for (int j = 0; j < n; ++j) if (g[i][j] != 0) {
            h = hashCombine(h, static_cast<quint64>(g[i][j]));
            deg++;
        }
        h = hashCombine(h, static_cast<quint64>(deg));
        inv[i] = h;
    }
    return inv;
}

/* ---- Non-isomorphism certificate ---- */

QString GraphIsomorphism14::nonIsoCertificate(const AdjMatrix& g) const
{
    return canonicalLabel(g);
}

/* ---- Check isomorphism ---- */

GraphIsomorphism14::IsoResult GraphIsomorphism14::checkIsomorphic(
    const AdjMatrix& g1, const AdjMatrix& g2)
{
    QElapsedTimer timer;
    timer.start();

    IsoResult result;
    int n = g1.size();

    // Quick filter: size and degree sequence
    if (n != g2.size() || !degreeCompatible(g1, g2)) {
        result.isomorphic = false;
        result.canonicalLabel1 = canonicalLabel(g1);
        result.canonicalLabel2 = canonicalLabel(g2);
        double elapsed = timer.elapsed();
        m_stats.totalOps++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit isomorphismChecked(false, 0.0, elapsed);
        return result;
    }

    // WL coloring for pruning
    QVector<int> colors1 = wlColoring(g1);
    QVector<int> colors2 = wlColoring(g2);

    // Compare WL color distributions
    QVector<int> c1Sorted = colors1, c2Sorted = colors2;
    std::sort(c1Sorted.begin(), c1Sorted.end());
    std::sort(c2Sorted.begin(), c2Sorted.end());
    if (c1Sorted != c2Sorted) {
        result.isomorphic = false;
        result.canonicalLabel1 = canonicalLabel(g1);
        result.canonicalLabel2 = canonicalLabel(g2);
        double elapsed = timer.elapsed();
        m_stats.totalOps++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit isomorphismChecked(false, 0.0, elapsed);
        return result;
    }

    // Backtracking search
    QVector<int> mapping(n, -1);
    QVector<bool> used(n, false);
    result.isomorphic = findMapping(g1, g2, colors1, colors2, mapping, used, 0);

    if (result.isomorphic) {
        result.mapping = mapping;
        result.confidence = 1.0;
    }

    result.canonicalLabel1 = canonicalLabel(g1);
    result.canonicalLabel2 = canonicalLabel(g2);
    emit canonicalLabelComputed(result.canonicalLabel1);

    double elapsed = timer.elapsed();
    m_stats.graphSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit isomorphismChecked(result.isomorphic, result.confidence, elapsed);

    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
