/**
 * @file EdgeColoring6.cpp
 * @brief EdgeColoring6 实现
 *
 * 实现边着色：Misra-Gries边访问序与扇形旋转至多Δ+1色。
 */

#include "utils/graph256/EdgeColoring6.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EdgeColoring6::EdgeColoring6(QObject *parent) : QObject(parent) {}
EdgeColoring6::~EdgeColoring6() = default;

/* ---- Set edges ---- */

void EdgeColoring6::setEdges(const QVector<QPair<int, int>>& edges, int numVertices)
{
    m_n = numVertices;
    m_edges.resize(edges.size());
    for (int i = 0; i < edges.size(); ++i) {
        m_edges[i].u = edges[i].first;
        m_edges[i].v = edges[i].second;
        m_edges[i].color = -1;
    }
    buildAdjacency();
}

/* ---- Build adjacency ---- */

void EdgeColoring6::buildAdjacency()
{
    m_adj.resize(m_n);
    for (auto& list : m_adj) list.clear();

    for (int i = 0; i < m_edges.size(); ++i) {
        m_adj[m_edges[i].u].append(i);
        m_adj[m_edges[i].v].append(i);
    }
}

/* ---- Max degree ---- */

int EdgeColoring6::maxDegree() const
{
    int maxDeg = 0;
    for (const auto& list : m_adj)
        maxDeg = qMax(maxDeg, list.size());
    return maxDeg;
}

/* ---- Free color at vertex ---- */

int EdgeColoring6::freeColor(int v) const
{
    int maxC = maxDegree() + 1;
    for (int c = 0; c <= maxC; ++c) {
        bool used = false;
        for (int ei : m_adj[v]) {
            if (m_edges[ei].color == c) { used = true; break; }
        }
        if (!used) return c;
    }
    return maxC;
}

/* ---- Neighbor on color ---- */

int EdgeColoring6::neighborOnColor(int v, int c) const
{
    for (int ei : m_adj[v]) {
        if (m_edges[ei].color == c) {
            return (m_edges[ei].u == v) ? m_edges[ei].v : m_edges[ei].u;
        }
    }
    return -1;
}

/* ---- Build maximal fan ---- */

QVector<int> EdgeColoring6::buildFan(int pivot, int edgeIdx) const
{
    QVector<int> fan;
    fan.append(edgeIdx);

    int target = m_edges[edgeIdx].u;
    if (target == pivot) target = m_edges[edgeIdx].v;

    int maxC = maxDegree() + 1;
    bool extended = true;
    while (extended) {
        extended = false;
        for (int ei : m_adj[pivot]) {
            if (m_edges[ei].color >= 0) continue;  // Already colored
            int neighbor = (m_edges[ei].u == pivot) ? m_edges[ei].v : m_edges[ei].u;
            // Check if neighbor has a color used by the target of the last fan edge
            int lastFanIdx = fan.last();
            int lastTarget = (m_edges[lastFanIdx].u == pivot) ? m_edges[lastFanIdx].v : m_edges[lastFanIdx].u;
            bool canExtend = false;
            for (int c = 0; c <= maxC; ++c) {
                // Check if this color is free at lastTarget
                bool freeAtLast = true;
                for (int fei : m_adj[lastTarget])
                    if (m_edges[fei].color == c) { freeAtLast = false; break; }
                // And used at neighbor
                bool usedAtNbr = false;
                for (int fei : m_adj[neighbor])
                    if (m_edges[fei].color == c) { usedAtNbr = true; break; }
                if (freeAtLast && usedAtNbr) { canExtend = true; break; }
            }
            if (canExtend) { fan.append(ei); extended = true; break; }
        }
    }
    return fan;
}

/* ---- Rotate fan colors ---- */

void EdgeColoring6::rotateFan(const QVector<int>& fan, int cdash)
{
    // Shift colors: fan[i+1] gets fan[i]'s color, first gets cdash
    for (int i = fan.size() - 1; i > 0; --i)
        m_edges[fan[i]].color = m_edges[fan[i - 1]].color;
    m_edges[fan[0]].color = cdash;
}

/* ---- Invert alternating path ---- */

void EdgeColoring6::invertPath(int v, int c1, int c2)
{
    while (true) {
        int next = neighborOnColor(v, c1);
        // Swap c1 <-> c2 at v
        for (int ei : m_adj[v]) {
            if (m_edges[ei].color == c1) m_edges[ei].color = c2;
            else if (m_edges[ei].color == c2) m_edges[ei].color = c1;
        }
        if (next < 0) break;
        v = next;
        // Swap c1 <-> c2 at v
        for (int ei : m_adj[v]) {
            if (m_edges[ei].color == c1) m_edges[ei].color = c2;
            else if (m_edges[ei].color == c2) m_edges[ei].color = c1;
        }
    }
}

/* ---- Color (Misra-Gries) ---- */

QVector<EdgeColoring6::Edge> EdgeColoring6::color()
{
    QElapsedTimer timer;
    timer.start();

    int maxC = maxDegree() + 1;
    m_vertexColor.resize(m_n);
    for (auto& row : m_vertexColor) {
        row.resize(maxC + 1, -1);
    }

    // Reset colors
    for (auto& e : m_edges) e.color = -1;

    for (int ei = 0; ei < m_edges.size(); ++ei) {
        int u = m_edges[ei].u;
        int v = m_edges[ei].v;

        int cu = freeColor(u);
        int cv = freeColor(v);

        if (cu == cv) {
            // Both have same free color: simply assign
            m_edges[ei].color = cu;
        } else {
            // Invert maximal alternating path from v on colors cu and cv
            invertPath(v, cu, cv);
            // Now cv is free at v, cu is free at u
            // Build fan from u through uncolored edges
            QVector<int> fan = buildFan(u, ei);
            int cdash = freeColor(v);
            rotateFan(fan, cdash);
        }

        emit edgeColored(ei, m_edges[ei].color);
    }

    // Count colors used
    int maxUsed = 0;
    for (const auto& e : m_edges)
        if (e.color > maxUsed) maxUsed = e.color;

    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.maxDegree = maxDegree();
    m_stats.colorsUsed = maxUsed + 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coloringCompleted(maxUsed + 1, m_stats.maxDegree, timer.elapsed());
    return m_edges;
}

/* ---- Verify coloring ---- */

bool EdgeColoring6::verifyColoring(const QVector<Edge>& colored) const
{
    for (int i = 0; i < colored.size(); ++i) {
        if (colored[i].color < 0) return false;
        for (int j = i + 1; j < colored.size(); ++j) {
            if (colored[i].color == colored[j].color) {
                // Same color: check if edges share a vertex
                if (colored[i].u == colored[j].u || colored[i].u == colored[j].v ||
                    colored[i].v == colored[j].u || colored[i].v == colored[j].v)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Reset ---- */

void EdgeColoring6::resetStatistics()
{
    m_edges.clear(); m_adj.clear(); m_vertexColor.clear();
    m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
