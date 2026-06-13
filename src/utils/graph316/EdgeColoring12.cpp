/**
 * @file EdgeColoring12.cpp
 * @brief EdgeColoring12 实现
 *
 * 实现边着色：Misra-Gries边访问排序与最大匹配增广实现最多Delta+1色边着色。
 */

#include "utils/graph316/EdgeColoring12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring12::EdgeColoring12(QObject *parent)
    : QObject(parent) {}

EdgeColoring12::~EdgeColoring12() = default;

/* ---- Set graph ---- */

void EdgeColoring12::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = qMax(1, numVertices);
    m_edges = edges;

    // Build adjacency list: vertex -> incident edge indices
    m_adj.resize(m_numVertices);
    for (auto& list : m_adj) list.clear();

    for (int i = 0; i < m_edges.size(); ++i) {
        int u = qBound(0, m_edges[i].from, m_numVertices - 1);
        int v = qBound(0, m_edges[i].to, m_numVertices - 1);
        m_adj[u].append(i);
        m_adj[v].append(i);
    }
}

/* ---- Maximum degree ---- */

int EdgeColoring12::maxDegree() const
{
    int delta = 0;
    for (const auto& list : m_adj)
        delta = qMax(delta, list.size());
    return delta;
}

/* ---- Find free color at vertex ---- */

int EdgeColoring12::freeColor(int vertex, const QVector<int>& edgeColors) const
{
    if (vertex < 0 || vertex >= m_numVertices) return 0;
    int delta = maxDegree();

    // Find smallest color not used by incident edges at this vertex
    QVector<bool> used(delta + 2, false);
    for (int ei : m_adj[vertex]) {
        if (edgeColors[ei] >= 0 && edgeColors[ei] < used.size())
            used[edgeColors[ei]] = true;
    }
    for (int c = 0; c <= delta + 1; ++c)
        if (!used[c]) return c;
    return delta + 1;
}

/* ---- Missing color at vertex ---- */

int EdgeColoring12::missingColor(int vertex, const QVector<int>& edgeColors) const
{
    return freeColor(vertex, edgeColors);
}

/* ---- Build maximal fan ---- */

QVector<int> EdgeColoring12::buildFan(int u, int v, const QVector<int>& edgeColors) const
{
    QVector<int> fan;

    // Start with edge (u, v)
    int startEdge = -1;
    for (int ei : m_adj[u]) {
        int other = (m_edges[ei].from == u) ? m_edges[ei].to : m_edges[ei].from;
        if (other == v) { startEdge = ei; break; }
    }
    if (startEdge < 0) return fan;
    fan.append(startEdge);

    // Extend fan: find edges from u whose color equals missing color of previous vertex
    bool extended = true;
    while (extended) {
        extended = false;
        int lastEdge = fan.last();
        int lastV = (m_edges[lastEdge].from == u) ? m_edges[lastEdge].to : m_edges[lastEdge].from;
        int needed = missingColor(lastV, edgeColors);

        for (int ei : m_adj[u]) {
            if (fan.contains(ei)) continue;
            int other = (m_edges[ei].from == u) ? m_edges[ei].to : m_edges[ei].from;
            if (edgeColors[ei] == needed) {
                fan.append(ei);
                extended = true;
                break;
            }
        }
    }
    return fan;
}

/* ---- Invert CD-path (swap colors c and d along alternating path) ---- */

void EdgeColoring12::invertPath(int start, int c, int d, QVector<int>& edgeColors)
{
    int current = start;
    while (current >= 0 && current < m_numVertices) {
        bool found = false;
        for (int ei : m_adj[current]) {
            if (edgeColors[ei] == c) {
                edgeColors[ei] = d;
                int next = (m_edges[ei].from == current) ? m_edges[ei].to : m_edges[ei].from;
                current = next;
                // Swap c and d for next step
                std::swap(c, d);
                found = true;
                break;
            }
        }
        if (!found) break;
    }
}

/* ---- Rotate fan and assign color ---- */

void EdgeColoring12::rotateFan(const QVector<int>& fan, int freeCol, QVector<int>& edgeColors)
{
    // Shift colors: fan[i] gets color of fan[i+1], last gets freeCol
    for (int i = fan.size() - 1; i > 0; --i)
        edgeColors[fan[i]] = edgeColors[fan[i - 1]];
    edgeColors[fan[0]] = freeCol;
}

/* ---- Main Misra-Gries coloring ---- */

EdgeColoring12::ColorResult EdgeColoring12::color()
{
    QElapsedTimer timer;
    timer.start();

    ColorResult result;
    int E = m_edges.size();
    if (E == 0 || m_numVertices == 0) return result;

    int delta = maxDegree();
    result.maxDegree = delta;

    // Initialize all edges as uncolored
    QVector<int> edgeColors(E, -1);

    // Process edges in Misra-Gries visit order
    for (int ei = 0; ei < E; ++ei) {
        int u = m_edges[ei].from;
        int v = m_edges[ei].to;

        // Find free colors at u and v
        int cu = freeColor(u, edgeColors);
        int cv = freeColor(v, edgeColors);

        if (cu == cv) {
            // Simple case: same free color at both endpoints
            edgeColors[ei] = cu;
        } else {
            // Build maximal fan from u through v
            QVector<int> fan = buildFan(u, v, edgeColors);
            if (fan.isEmpty()) {
                edgeColors[ei] = cu;
                continue;
            }

            int lastV = (m_edges[fan.last()].from == u) ? m_edges[fan.last()].to : m_edges[fan.last()].from;
            int cd = missingColor(lastV, edgeColors);

            // Invert CD-path starting from u with colors cv and cd
            invertPath(u, cv, cd, edgeColors);

            // Find new free color at u
            int freeCol = freeColor(u, edgeColors);

            // Rotate fan and assign
            // Rebuild fan after inversion
            fan = buildFan(u, v, edgeColors);
            rotateFan(fan, freeCol, edgeColors);
        }
    }

    // Copy colors to result
    result.edges = m_edges;
    for (int i = 0; i < E; ++i)
        result.edges[i].color = edgeColors[i];

    // Count colors used
    int maxColor = 0;
    for (int c : edgeColors)
        maxColor = qMax(maxColor, c);
    result.numColors = maxColor + 1;
    result.valid = verify(result.edges);
    result.elapsedMs = timer.elapsed();

    m_stats.totalColorings++;
    m_stats.maxColorsUsed = qMax(m_stats.maxColorsUsed, result.numColors);
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringDone(result.numColors, E, result.elapsedMs);
    return result;
}

/* ---- Verify coloring ---- */

bool EdgeColoring12::verify(const QVector<Edge>& colored) const
{
    // Check: no two adjacent edges share a color
    for (int i = 0; i < colored.size(); ++i) {
        if (colored[i].color < 0) return false;
        for (int j = i + 1; j < colored.size(); ++j) {
            if (colored[i].color == colored[j].color) {
                // Adjacent if they share a vertex
                if (colored[i].from == colored[j].from ||
                    colored[i].from == colored[j].to ||
                    colored[i].to == colored[j].from ||
                    colored[i].to == colored[j].to)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Reset ---- */

void EdgeColoring12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_edges.clear();
    m_adj.clear();
}
