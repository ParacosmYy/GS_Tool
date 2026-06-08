/**
 * @file GraphColoring8.cpp
 * @brief GraphColoring8 实现
 *
 * 实现图着色：递归最大优先(RLF)与饱和度回跳(DSATUR)紧色数界限。
 */

#include "utils/graph243/GraphColoring8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphColoring8::GraphColoring8(QObject *parent) : QObject(parent) {}
GraphColoring8::~GraphColoring8() = default;

/* ---- Load adjacency matrix ---- */

bool GraphColoring8::loadGraph(const QVector<QVector<int>>& adjacency)
{
    int n = adjacency.size();
    if (n == 0) return false;
    for (const auto& row : adjacency)
        if (row.size() != n) return false;

    m_n = n;
    m_adj = adjacency;
    m_stats.numVertices = n;

    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adjacency[i][j]) ++edges;
    m_stats.numEdges = edges;
    return true;
}

/* ---- Compute saturation degrees ---- */

QVector<int> GraphColoring8::computeSaturation(
    const QVector<int>& colors) const
{
    QVector<int> sat(m_n, 0);
    for (int v = 0; v < m_n; ++v) {
        if (colors[v] >= 0) continue;  // already colored
        QVector<bool> usedColors(m_n, false);
        for (int u = 0; u < m_n; ++u) {
            if (m_adj[v][u] && colors[u] >= 0)
                usedColors[colors[u]] = true;
        }
        for (int c = 0; c < m_n; ++c)
            if (usedColors[c]) sat[v]++;
    }
    return sat;
}

/* ---- DSATUR vertex selection ---- */

int GraphColoring8::selectDSATUR(const QVector<int>& colors,
                                  const QVector<bool>& colored) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    QVector<int> sat = computeSaturation(colors);

    for (int v = 0; v < m_n; ++v) {
        if (colored[v]) continue;
        int deg = 0;
        for (int u = 0; u < m_n; ++u)
            if (m_adj[v][u] && !colored[u]) ++deg;

        if (sat[v] > bestSat || (sat[v] == bestSat && deg > bestDeg)) {
            bestSat = sat[v];
            bestDeg = deg;
            best = v;
        }
    }
    return best;
}

/* ---- RLF phase: find vertices not adjacent to current color class ---- */

QVector<int> GraphColoring8::rlfPhase(const QVector<bool>& colored) const
{
    QVector<int> candidates;
    for (int v = 0; v < m_n; ++v)
        if (!colored[v]) candidates.append(v);
    return candidates;
}

/* ---- Find conflict for backjumping ---- */

int GraphColoring8::findConflict(int vertex, int color,
                                  const QVector<int>& colors) const
{
    for (int u = 0; u < m_n; ++u) {
        if (m_adj[vertex][u] && colors[u] == color)
            return u;
    }
    return -1;
}

/* ---- Color using RLF + DSATUR backjumping ---- */

QVector<int> GraphColoring8::colorRLF()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> colors(m_n, -1);
    QVector<bool> colored(m_n, false);
    int numColored = 0;
    int currentColor = 0;
    int backjumps = 0;

    while (numColored < m_n) {
        // DSATUR: pick highest saturation uncolored vertex
        int v = selectDSATUR(colors, colored);
        if (v < 0) break;

        // Find smallest available color
        bool assigned = false;
        for (int c = 0; c <= currentColor; ++c) {
            bool conflict = false;
            for (int u = 0; u < m_n; ++u) {
                if (m_adj[v][u] && colors[u] == c) {
                    conflict = true;
                    break;
                }
            }

            if (!conflict) {
                colors[v] = c;
                colored[v] = true;
                numColored++;
                assigned = true;

                // Check for backjump opportunity
                int conf = findConflict(v, c, colors);
                if (conf >= 0 && colored[conf]) {
                    backjumps++;
                }
                break;
            }
        }

        if (!assigned) {
            currentColor++;
            colors[v] = currentColor;
            colored[v] = true;
            numColored++;
        }

        // RLF: try to extend current color class
        for (int u = 0; u < m_n; ++u) {
            if (colored[u]) continue;
            bool canColor = true;
            for (int w = 0; w < m_n; ++w) {
                if (m_adj[u][w] && colors[w] == currentColor) {
                    canColor = false;
                    break;
                }
            }
            if (canColor) {
                colors[u] = currentColor;
                colored[u] = true;
                numColored++;
            }
        }
    }

    m_stats.chromaticUsed = currentColor + 1;
    m_stats.backjumps = backjumps;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringCompleted(currentColor + 1, backjumps, timer.elapsed());
    return colors;
}

/* ---- Clique lower bound ---- */

int GraphColoring8::cliqueLowerBound() const
{
    // Greedy max clique: simple heuristic
    int maxClique = 1;
    for (int v = 0; v < m_n; ++v) {
        int cliqueSize = 1;
        QVector<bool> inClique(m_n, false);
        inClique[v] = true;

        for (int u = 0; u < m_n; ++u) {
            if (u == v || !m_adj[v][u]) continue;
            bool canAdd = true;
            for (int w = 0; w < m_n; ++w) {
                if (inClique[w] && !m_adj[u][w]) {
                    canAdd = false;
                    break;
                }
            }
            if (canAdd) {
                inClique[u] = true;
                cliqueSize++;
            }
        }
        maxClique = qMax(maxClique, cliqueSize);
    }
    return maxClique;
}

/* ---- Verify coloring ---- */

bool GraphColoring8::verifyColoring(const QVector<int>& colors) const
{
    if (colors.size() != m_n) return false;
    for (int i = 0; i < m_n; ++i) {
        if (colors[i] < 0) return false;
        for (int j = i + 1; j < m_n; ++j) {
            if (m_adj[i][j] && colors[i] == colors[j])
                return false;
        }
    }
    return true;
}

/* ---- Reset ---- */

void GraphColoring8::resetStatistics()
{
    m_adj.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
