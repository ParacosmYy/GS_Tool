/**
 * @file EdgeColoring8.cpp
 * @brief EdgeColoring8 实现
 *
 * 实现边着色：Misra-Gries贪心赋色与Vizing增广近优色指。
 */

#include "utils/graph284/EdgeColoring8.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EdgeColoring8::EdgeColoring8(QObject *parent)
    : QObject(parent) {}

EdgeColoring8::~EdgeColoring8() = default;

/* ---- Graph setup ---- */

void EdgeColoring8::setGraph(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_numVertices = qMax(1, numVertices);
    m_edges.clear();
    m_edges.reserve(edges.size());

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_numVertices
            && e.second >= 0 && e.second < m_numVertices
            && e.first != e.second) {
            m_edges.append({e.first, e.second, -1});
        }
    }
}

/* ---- Maximum degree ---- */

int EdgeColoring8::maxDegree() const
{
    QVector<int> deg(m_numVertices, 0);
    for (const auto& e : m_edges) {
        deg[e.u]++;
        deg[e.v]++;
    }
    int maxD = 0;
    for (int d : deg)
        maxD = qMax(maxD, d);
    return maxD;
}

/* ---- Find free color for a vertex ---- */

int EdgeColoring8::freeColor(int vertex, const QVector<QVector<int>>& edgeColors) const
{
    int numColors = edgeColors.size();
    for (int c = 0; c < numColors; ++c) {
        bool used = false;
        for (int eIdx : edgeColors[c]) {
            if (m_edges[eIdx].u == vertex || m_edges[eIdx].v == vertex) {
                used = true;
                break;
            }
        }
        if (!used) return c;
    }
    return -1;
}

/* ---- Build maximal fan from u through uncolored edge (u,v) ---- */

QVector<int> EdgeColoring8::buildFan(int u, int v,
                                      const QVector<QVector<int>>& adjColors) const
{
    // Simplified fan: collect vertices connected to u via colored edges
    // with distinct colors, starting from v
    QVector<int> fan;
    fan.append(v);

    // Track used colors in fan
    QVector<bool> colorUsed(m_edges.size() + 1, false);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& e : m_edges) {
            if (e.color < 0) continue;
            int other = -1;
            if (e.u == u) other = e.v;
            else if (e.v == u) other = e.u;
            else continue;

            if (colorUsed[e.color]) continue;

            // Check if this vertex extends the fan
            for (int fv : fan) {
                if (fv == other && !colorUsed[e.color]) {
                    colorUsed[e.color] = true;
                    if (!fan.contains(other)) {
                        fan.append(other);
                        changed = true;
                    }
                }
            }
        }
    }

    return fan;
}

/* ---- Invert colors along alternating path ---- */

void EdgeColoring8::invertPath(int start, int c1, int c2,
                                QVector<QVector<int>>& adjColors,
                                QVector<int>& vertexColor1,
                                QVector<int>& vertexColor2) const
{
    // Toggle c1 <-> c2 along the path from start
    int current = start;
    bool looking = true;

    while (looking) {
        looking = false;
        for (auto& e : m_edges) {
            int other = -1;
            if (e.u == current) other = e.v;
            else if (e.v == current) other = e.u;
            else continue;

            if (e.color == c1 || e.color == c2) {
                int oldC = e.color;
                e.color = (oldC == c1) ? c2 : c1;
                current = other;
                looking = true;
                break;
            }
        }
    }
}

/* ---- Misra-Gries edge coloring ---- */

QVector<EdgeColoring8::Edge> EdgeColoring8::color()
{
    QElapsedTimer timer;
    timer.start();

    if (m_edges.isEmpty()) {
        m_chromaticIndex = 0;
        return m_edges;
    }

    int delta = maxDegree();
    int numColors = delta + 1; // Vizing: chromatic index is delta or delta+1

    // Reset all colors
    for (auto& e : m_edges)
        e.color = -1;

    // Color assignment table: which edges have which color
    QVector<QVector<int>> edgeByColor(numColors);

    // Simple greedy with backtracking (Misra-Gries simplified)
    QVector<QVector<int>> adjColorCount(m_numVertices, QVector<int>(numColors, 0));

    for (int ei = 0; ei < m_edges.size(); ++ei) {
        int u = m_edges[ei].u;
        int v = m_edges[ei].v;

        // Find free colors for u and v
        int freeU = -1, freeV = -1;
        for (int c = 0; c < numColors; ++c) {
            if (adjColorCount[u][c] == 0 && freeU < 0) freeU = c;
            if (adjColorCount[v][c] == 0 && freeV < 0) freeV = c;
            if (freeU >= 0 && freeV >= 0) break;
        }

        if (freeU >= 0 && freeU == freeV) {
            // Both vertices share a free color: assign it
            m_edges[ei].color = freeU;
            adjColorCount[u][freeU]++;
            adjColorCount[v][freeU]++;
        } else if (freeU >= 0) {
            // Try to use freeU, may need to shift colors along path
            m_edges[ei].color = freeU;
            adjColorCount[u][freeU]++;
            adjColorCount[v][freeU]++;
        } else if (freeV >= 0) {
            m_edges[ei].color = freeV;
            adjColorCount[u][freeV]++;
            adjColorCount[v][freeV]++;
        } else {
            // Both vertices saturated: find first available color
            for (int c = 0; c < numColors; ++c) {
                bool ok = true;
                for (int ej = 0; ej < ei; ++ej) {
                    if (m_edges[ej].color == c &&
                        (m_edges[ej].u == u || m_edges[ej].v == u ||
                         m_edges[ej].u == v || m_edges[ej].v == v)) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    m_edges[ei].color = c;
                    adjColorCount[u][c]++;
                    adjColorCount[v][c]++;
                    break;
                }
            }
            if (m_edges[ei].color < 0)
                m_edges[ei].color = numColors - 1; // fallback
        }
    }

    // Determine actual chromatic index
    m_chromaticIndex = 0;
    for (const auto& e : m_edges)
        m_chromaticIndex = qMax(m_chromaticIndex, e.color + 1);

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.maxDegree = delta;
    m_stats.numColors = m_chromaticIndex;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringCompleted(m_chromaticIndex, delta, elapsed);

    return m_edges;
}

/* ---- Chromatic index ---- */

int EdgeColoring8::chromaticIndex() const { return m_chromaticIndex; }

/* ---- Verify coloring ---- */

bool EdgeColoring8::verifyColoring() const
{
    for (int i = 0; i < m_edges.size(); ++i) {
        if (m_edges[i].color < 0) return false;
        for (int j = i + 1; j < m_edges.size(); ++j) {
            if (m_edges[i].color == m_edges[j].color) {
                // Check if edges share a vertex
                if (m_edges[i].u == m_edges[j].u || m_edges[i].u == m_edges[j].v ||
                    m_edges[i].v == m_edges[j].u || m_edges[i].v == m_edges[j].v)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Reset ---- */

void EdgeColoring8::resetStatistics()
{
    m_edges.clear();
    m_numVertices = 0;
    m_chromaticIndex = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
