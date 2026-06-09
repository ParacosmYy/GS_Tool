/**
 * @file GraphColoring9.cpp
 * @brief GraphColoring9 实现
 *
 * 实现图着色：DSatur饱和度排序与前向检查回溯搜索。
 */

#include "utils/graph257/GraphColoring9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphColoring9::GraphColoring9(QObject *parent) : QObject(parent) {}
GraphColoring9::~GraphColoring9() = default;

/* ---- Configuration ---- */

void GraphColoring9::setMaxColors(int max) { m_maxColors = qMax(0, max); }

void GraphColoring9::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_numVertices = adjacency.size();
}

void GraphColoring9::setEdges(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_numVertices = numVertices;
    m_adj.resize(numVertices);
    for (auto& row : m_adj) row.clear();

    int edgeCount = 0;
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices &&
            e.second >= 0 && e.second < numVertices) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
            edgeCount++;
        }
    }
    m_stats.numEdges = edgeCount;
}

/* ---- Saturation degree ---- */

int GraphColoring9::saturationDegree(int vertex) const
{
    // Count distinct colors among neighbors
    QVector<bool> seen(m_numVertices + 1, false);
    int count = 0;
    for (int nb : m_adj[vertex]) {
        int c = m_colors[nb];
        if (c >= 0 && c < seen.size() && !seen[c]) {
            seen[c] = true;
            count++;
        }
    }
    return count;
}

/* ---- DSatur greedy for upper bound ---- */

int GraphColoring9::dsaturGreedy()
{
    m_colors.fill(-1);
    int n = m_numVertices;
    if (n == 0) return 0;

    int maxColor = 0;

    for (int step = 0; step < n; ++step) {
        // Pick uncolored vertex with highest saturation degree, tie-break by degree
        int best = -1;
        int bestSat = -1;
        int bestDeg = -1;
        for (int v = 0; v < n; ++v) {
            if (m_colors[v] >= 0) continue;
            int sat = saturationDegree(v);
            int deg = m_adj[v].size();
            if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
                bestSat = sat;
                bestDeg = deg;
                best = v;
            }
        }
        if (best < 0) break;

        // Find smallest unused color
        QVector<bool> used(n + 1, false);
        for (int nb : m_adj[best]) {
            if (m_colors[nb] >= 0 && m_colors[nb] < used.size())
                used[m_colors[nb]] = true;
        }
        int c = 0;
        while (c < used.size() && used[c]) c++;
        m_colors[best] = c;
        maxColor = qMax(maxColor, c);
    }

    return maxColor + 1;
}

/* ---- Select next vertex by DSatur for backtracking ---- */

int GraphColoring9::selectVertex(const QVector<QVector<bool>>& domains) const
{
    int best = -1;
    int bestSat = -1;
    int bestDomainSize = std::numeric_limits<int>::max();
    int bestDeg = -1;

    for (int v = 0; v < m_numVertices; ++v) {
        if (m_colors[v] >= 0) continue;
        int sat = saturationDegree(v);
        // Domain size: remaining valid colors
        int domSize = 0;
        if (v < domains.size())
            for (bool b : domains[v]) if (b) domSize++;
        int deg = m_adj[v].size();

        // Choose highest saturation, smallest domain (MRV tie-break), highest degree
        if (sat > bestSat ||
            (sat == bestSat && domSize < bestDomainSize) ||
            (sat == bestSat && domSize == bestDomainSize && deg > bestDeg)) {
            bestSat = sat;
            bestDomainSize = domSize;
            bestDeg = deg;
            best = v;
        }
    }
    return best;
}

/* ---- Backtracking with forward checking ---- */

bool GraphColoring9::backtrack(int depth, int maxC, QVector<QVector<bool>>& domains)
{
    // All colored?
    bool allColored = true;
    for (int i = 0; i < m_numVertices; ++i)
        if (m_colors[i] < 0) { allColored = false; break; }
    if (allColored) return true;

    int v = selectVertex(domains);
    if (v < 0) return true;

    // Try each color in domain
    if (v < domains.size()) {
        for (int c = 0; c < maxC; ++c) {
            if (!domains[v][c]) continue;

            m_colors[v] = c;

            // Forward checking: prune neighbor domains
            QVector<QPair<int, int>> pruned;
            bool feasible = true;
            for (int nb : m_adj[v]) {
                if (m_colors[nb] >= 0) continue;
                if (nb < domains.size() && domains[nb][c]) {
                    domains[nb][c] = false;
                    pruned.append({nb, c});
                    // Check if domain became empty
                    bool hasColor = false;
                    for (int cc = 0; cc < maxC; ++cc)
                        if (domains[nb][cc]) { hasColor = true; break; }
                    if (!hasColor) { feasible = false; break; }
                }
            }

            if (feasible && backtrack(depth + 1, maxC, domains))
                return true;

            // Restore pruned domains
            for (const auto& p : pruned)
                domains[p.first][p.second] = true;
            m_colors[v] = -1;
            m_stats.backtracks++;
        }
    }

    return false;
}

/* ---- Color ---- */

QVector<int> GraphColoring9::color()
{
    QElapsedTimer timer;
    timer.start();

    m_colors.fill(-1);
    m_stats.backtracks = 0;

    if (m_numVertices == 0) return m_colors;

    // Step 1: DSatur greedy for upper bound
    int greedyColors = dsaturGreedy();
    auto greedyResult = m_colors;

    // Step 2: Try to improve with backtracking from chromatic number downward
    int bestColors = greedyColors;
    auto bestResult = greedyResult;

    int startColor = (m_maxColors > 0) ? m_maxColors : greedyColors;
    int minColor = qMax(1, startColor - 3);

    for (int tryC = qMin(startColor, greedyColors); tryC >= minColor; --tryC) {
        m_colors.fill(-1);
        QVector<QVector<bool>> domains(m_numVertices, QVector<bool>(tryC, true));
        m_stats.backtracks = 0;

        if (backtrack(0, tryC, domains)) {
            bestColors = tryC;
            bestResult = m_colors;
        } else {
            break; // Can't improve further
        }
    }

    m_colors = bestResult;

    m_stats.numVertices = m_numVertices;
    m_stats.colorsUsed = chromaticNumber();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coloringCompleted(m_stats.colorsUsed, m_stats.backtracks, timer.elapsed());
    return m_colors;
}

/* ---- Validation ---- */

bool GraphColoring9::isValid() const
{
    for (int v = 0; v < m_numVertices; ++v) {
        if (m_colors[v] < 0) return false;
        for (int nb : m_adj[v]) {
            if (m_colors[v] == m_colors[nb]) return false;
        }
    }
    return true;
}

/* ---- Chromatic number ---- */

int GraphColoring9::chromaticNumber() const
{
    int maxC = 0;
    for (int c : m_colors) maxC = qMax(maxC, c);
    return maxC + 1;
}

/* ---- Reset ---- */

void GraphColoring9::resetStatistics()
{
    m_adj.clear(); m_colors.clear();
    m_numVertices = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
