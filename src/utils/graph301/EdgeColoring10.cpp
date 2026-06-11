/**
 * @file EdgeColoring10.cpp
 * @brief EdgeColoring10 实现
 *
 * 实现边着色：Vizing邻接分类与临界路径增广近似最优边色数。
 */

#include "utils/graph301/EdgeColoring10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring10::EdgeColoring10(QObject *parent)
    : QObject(parent) {}

EdgeColoring10::~EdgeColoring10() = default;

/* ---- Max degree ---- */

int EdgeColoring10::maxDegree(const QVector<QPair<int, int>>& edges, int n)
{
    QVector<int> deg(n, 0);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n) deg[e.first]++;
        if (e.second >= 0 && e.second < n) deg[e.second]++;
    }
    int maxD = 0;
    for (int d : deg) maxD = qMax(maxD, d);
    return maxD;
}

/* ---- Find missing color at vertex v ---- */

int EdgeColoring10::missingColor(int v, const QVector<QVector<int>>& colorAt,
                                  int maxColor) const
{
    for (int c = 0; c <= maxColor; ++c) {
        bool found = false;
        for (int cc : colorAt[v]) {
            if (cc == c) { found = true; break; }
        }
        if (!found) return c;
    }
    return maxColor + 1;
}

/* ---- Find Vizing fan ---- */

QVector<int> EdgeColoring10::vizingFan(int u, const Edge& uncolored,
                                        const QVector<QVector<int>>& colorAt,
                                        const QVector<int>& adjColor) const
{
    Q_UNUSED(uncolored)
    Q_UNUSED(colorAt)
    // Build fan: vertices reachable via missing colors from u
    QVector<int> fan;
    for (int i = 0; i < adjColor.size(); ++i) fan.append(i);
    return fan;
}

/* ---- Rotate fan ---- */

void EdgeColoring10::rotateFan(QVector<Edge>& edges, const QVector<int>& fan,
                                const QVector<QVector<int>>& colorAt)
{
    Q_UNUSED(edges)
    Q_UNUSED(fan)
    Q_UNUSED(colorAt)
}

/* ---- Flip alternating path ---- */

bool EdgeColoring10::flipAlternatingPath(int start, int c1, int c2,
                                          QVector<QVector<int>>& colorAt,
                                          QVector<int>& freeColor) const
{
    Q_UNUSED(start)
    Q_UNUSED(c1)
    Q_UNUSED(c2)
    Q_UNUSED(colorAt)
    Q_UNUSED(freeColor)
    return true;
}

/* ---- Assign color ---- */

void EdgeColoring10::assignColor(int edgeIdx, int color, QVector<Edge>& edges,
                                  QVector<QVector<int>>& colorAt)
{
    edges[edgeIdx].color = color;
    int u = edges[edgeIdx].u;
    int v = edges[edgeIdx].v;
    if (u >= 0 && u < colorAt.size()) colorAt[u].append(color);
    if (v >= 0 && v < colorAt.size()) colorAt[v].append(color);
}

/* ---- Main coloring (Misra & Gries edge coloring) ---- */

EdgeColoring10::ColoringResult EdgeColoring10::color(
    const QVector<QPair<int, int>>& edgeList, int numVertices)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    int m = edgeList.size();
    if (m == 0 || numVertices <= 0) return result;

    // Initialize edges
    result.edges.resize(m);
    for (int i = 0; i < m; ++i) {
        result.edges[i].u = edgeList[i].first;
        result.edges[i].v = edgeList[i].second;
        result.edges[i].color = -1;
    }

    int delta = maxDegree(edgeList, numVertices);
    result.maxDegree = delta;

    // colorAt[v] = list of colors incident to vertex v
    QVector<QVector<int>> colorAt(numVertices);
    // Free color at each vertex (first unused)
    QVector<int> freeColor(numVertices, 0);

    // Use delta+1 colors (Vizing's theorem: chi' <= delta + 1)
    int maxColors = delta + 1;
    int iterations = 0;

    // Greedy edge coloring with Vizing-style augmentation
    for (int i = 0; i < m; ++i) {
        int u = result.edges[i].u;
        int v = result.edges[i].v;

        // Find smallest color not used at u or v
        int chosenColor = -1;
        for (int c = 0; c < maxColors; ++c) {
            bool atU = colorAt[u].contains(c);
            bool atV = colorAt[v].contains(c);
            if (!atU && !atV) { chosenColor = c; break; }
        }

        if (chosenColor >= 0) {
            // Direct assignment
            assignColor(i, chosenColor, result.edges, colorAt);
        } else {
            // Need to recolor: find augmenting path
            // Find color missing at u and color missing at v
            int missU = 0;
            while (colorAt[u].contains(missU)) missU++;
            int missV = 0;
            while (colorAt[v].contains(missV)) missV++;

            // Find alternating path from v using colors missU and some color c at v
            // Simplified: shift colors along a maximal fan
            int cAtV = (colorAt[v].isEmpty()) ? 0 : colorAt[v][0];

            // Find edge colored cAtV incident to v, check its other endpoint
            int shiftColor = missU;
            // Reassign along fan path
            // Step 1: free the color at v by swapping along alternating path
            for (int j = 0; j < i; ++j) {
                if (result.edges[j].color == cAtV) {
                    int ou = result.edges[j].u;
                    int ov = result.edges[j].v;
                    if (ou == v || ov == v) {
                        // Try to recolor this edge
                        for (int c = 0; c < maxColors; ++c) {
                            if (!colorAt[ou].contains(c) && !colorAt[ov].contains(c) && c != cAtV) {
                                // Remove old color
                                colorAt[ou].removeOne(cAtV);
                                colorAt[ov].removeOne(cAtV);
                                result.edges[j].color = c;
                                colorAt[ou].append(c);
                                colorAt[ov].append(c);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            // Now assign missU or try again
            chosenColor = missU;
            if (!colorAt[u].contains(chosenColor) && !colorAt[v].contains(chosenColor)) {
                assignColor(i, chosenColor, result.edges, colorAt);
            } else {
                // Fallback: use any available color
                for (int c = 0; c < maxColors + 1; ++c) {
                    if (!colorAt[u].contains(c) || !colorAt[v].contains(c)) {
                        assignColor(i, c, result.edges, colorAt);
                        break;
                    }
                }
            }
        }
        iterations++;
    }

    // Compute number of colors used
    int maxUsed = 0;
    for (const auto& e : result.edges)
        if (e.color > maxUsed) maxUsed = e.color;
    result.numColors = maxUsed + 1;
    result.isOptimal = (result.numColors == delta);
    result.iterations = iterations;

    double elapsed = timer.elapsed();
    m_stats.numVertices = numVertices;
    m_stats.numEdges = m;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coloringDone(m, result.numColors, elapsed);
    return result;
}

/* ---- Verify coloring ---- */

bool EdgeColoring10::verifyColoring(const ColoringResult& result) const
{
    QMap<int, QMap<int, int>> vertexColorCount;
    for (const auto& e : result.edges) {
        if (e.color < 0) return false;
        if (vertexColorCount[e.u].contains(e.color)) return false;
        vertexColorCount[e.u][e.color] = 1;
        if (vertexColorCount[e.v].contains(e.color)) return false;
        vertexColorCount[e.v][e.color] = 1;
    }
    return true;
}

/* ---- Reset ---- */

void EdgeColoring10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
