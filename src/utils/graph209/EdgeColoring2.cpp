/**
 * @file EdgeColoring2.cpp
 * @brief EdgeColoring2 实现
 *
 * 实现边着色：Misra-Gries算法、Vizing邻接表、类增广路径。
 */

#include "utils/graph209/EdgeColoring2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EdgeColoring2::EdgeColoring2(QObject *parent) : QObject(parent) {}
EdgeColoring2::~EdgeColoring2() = default;

/* ---- Free color at vertex ---- */

int EdgeColoring2::freeColor(int v, int maxCol) const
{
    QVector<bool> used(maxCol + 2, false);
    for (const auto& neighbor : m_adj[v])
        if (neighbor.second >= 0 && neighbor.second <= maxCol)
            used[neighbor.second] = true;
    for (int c = 0; c <= maxCol; ++c)
        if (!used[c]) return c;
    return maxCol + 1;
}

/* ---- Build fan ---- */

QVector<int> EdgeColoring2::buildFan(int x, int r, int maxCol) const
{
    QVector<int> fan;
    fan.append(r);

    // Greedily extend fan: find vertex v adjacent to x via uncolored edge
    // such that v shares a color with last vertex in fan
    bool extended = true;
    while (extended) {
        extended = false;
        int last = fan.back();
        // Find color of edge (x, last) if exists
        int lastColor = -1;
        for (const auto& nb : m_adj[x]) {
            if (nb.first == last) { lastColor = nb.second; break; }
        }
        if (lastColor < 0) break;

        // Find vertex w adjacent to x with same color
        for (const auto& nb : m_adj[x]) {
            if (nb.second == lastColor && !fan.contains(nb.first)) {
                fan.append(nb.first);
                extended = true;
                break;
            }
        }
    }
    return fan;
}

/* ---- Flip alternating path ---- */

void EdgeColoring2::flipPath(int x, int cd, int cf, int maxCol)
{
    // Find alternating path from x using colors cd and cf
    int cur = x;
    int color = cd;
    while (true) {
        bool found = false;
        for (auto& nb : m_adj[cur]) {
            if (nb.second == color) {
                nb.second = (color == cd) ? cf : cd;
                // Update reverse edge
                for (auto& rev : m_adj[nb.first]) {
                    if (rev.first == cur) { rev.second = nb.second; break; }
                }
                cur = nb.first;
                color = (color == cd) ? cf : cd;
                found = true;
                break;
            }
        }
        if (!found) break;
    }
}

/* ---- Rotate fan ---- */

void EdgeColoring2::rotateFan(int x, const QVector<int>& fan, int cd)
{
    // Shift colors along fan
    for (int i = fan.size() - 1; i > 0; --i) {
        // Find color of edge (x, fan[i-1])
        int prevColor = -1;
        for (auto& nb : m_adj[x]) {
            if (nb.first == fan[i-1]) { prevColor = nb.second; break; }
        }
        // Assign to edge (x, fan[i])
        for (auto& nb : m_adj[x]) {
            if (nb.first == fan[i]) {
                nb.second = prevColor;
                for (auto& rev : m_adj[fan[i]]) {
                    if (rev.first == x) { rev.second = prevColor; break; }
                }
                break;
            }
        }
    }
    // Assign cd to first fan edge
    for (auto& nb : m_adj[x]) {
        if (nb.first == fan[0]) {
            nb.second = cd;
            for (auto& rev : m_adj[fan[0]]) {
                if (rev.first == x) { rev.second = cd; break; }
            }
            break;
        }
    }
}

/* ---- Max degree ---- */

int EdgeColoring2::maxDegree(int numVertices,
                              const QVector<QPair<int, int>>& edges) const
{
    QVector<int> deg(numVertices, 0);
    for (const auto& e : edges) {
        deg[e.first]++;
        deg[e.second]++;
    }
    int maxDeg = 0;
    for (int d : deg) maxDeg = qMax(maxDeg, d);
    return maxDeg;
}

/* ---- Color ---- */

QVector<int> EdgeColoring2::color(const QVector<QPair<int, int>>& edges,
                                    int numVertices)
{
    QElapsedTimer timer;
    timer.start();

    int E = edges.size();
    if (E == 0 || numVertices <= 0) return {};

    int delta = maxDegree(numVertices, edges);
    int maxCol = delta + 1; // Vizing: at most delta+1 colors

    // Initialize adjacency list
    m_adj.assign(numVertices, QVector<QPair<int, int>>());
    m_edgeColor.assign(numVertices, QVector<int>(numVertices, -1));

    // Process edges one by one (Misra-Gries)
    QVector<int> resultColors(E, -1);

    for (int ei = 0; ei < E; ++ei) {
        int u = edges[ei].first;
        int v = edges[ei].second;

        // Find free colors at u and v
        int cu = freeColor(u, maxCol);
        int cv = freeColor(v, maxCol);

        if (cu == cv) {
            // Both have same free color: assign directly
            m_adj[u].append({v, cu});
            m_adj[v].append({u, cu});
            m_edgeColor[u][v] = cu;
            m_edgeColor[v][u] = cu;
            resultColors[ei] = cu;
        } else {
            // Find free color at u is cu, at v is cv
            // Flip alternating path from v using cu and cv
            flipPath(v, cu, cv, maxCol);

            // Now cv should be free at both u and v
            int newCv = freeColor(v, maxCol);
            m_adj[u].append({v, newCv});
            m_adj[v].append({u, newCv});
            m_edgeColor[u][v] = newCv;
            m_edgeColor[v][u] = newCv;
            resultColors[ei] = newCv;
        }
    }

    // Determine actual colors used
    int maxUsed = 0;
    for (int c : resultColors) maxUsed = qMax(maxUsed, c);
    m_colorsUsed = maxUsed + 1;

    m_stats.totalColorings++;
    m_stats.numVertices = numVertices;
    m_stats.numEdges = E;
    m_stats.colorsUsed = m_colorsUsed;
    m_stats.maxDegree = delta;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringCompleted(numVertices, E, m_colorsUsed, timer.elapsed());
    return resultColors;
}

/* ---- Edge color query ---- */

int EdgeColoring2::edgeColor(int u, int v) const
{
    if (u >= 0 && u < m_edgeColor.size() && v >= 0 && v < m_edgeColor[u].size())
        return m_edgeColor[u][v];
    return -1;
}

/* ---- Validation ---- */

bool EdgeColoring2::isValid(const QVector<QPair<int, int>>& edges,
                             const QVector<int>& colors) const
{
    // Check no two adjacent edges share a color
    for (int i = 0; i < edges.size(); ++i) {
        for (int j = i + 1; j < edges.size(); ++j) {
            if (colors[i] == colors[j]) {
                // Check if edges share a vertex
                if (edges[i].first == edges[j].first ||
                    edges[i].first == edges[j].second ||
                    edges[i].second == edges[j].first ||
                    edges[i].second == edges[j].second)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Reset ---- */

void EdgeColoring2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_colorsUsed = 0;
    m_adj.clear();
    m_edgeColor.clear();
}
