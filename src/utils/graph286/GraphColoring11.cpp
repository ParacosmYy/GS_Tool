/**
 * @file GraphColoring11.cpp
 * @brief GraphColoring11 实现
 *
 * 实现图着色：DSATUR饱和度排序与回溯传播色数最小化。
 */

#include "utils/graph286/GraphColoring11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphColoring11::GraphColoring11(QObject *parent)
    : QObject(parent) {}

GraphColoring11::~GraphColoring11() = default;

/* ---- Configuration ---- */

void GraphColoring11::setGraph(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_numVertices = qMax(0, numVertices);
    m_adjList.assign(m_numVertices, QVector<int>());

    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < m_numVertices &&
            edge.second >= 0 && edge.second < m_numVertices) {
            m_adjList[edge.first].append(edge.second);
            m_adjList[edge.second].append(edge.first);
        }
    }
}

void GraphColoring11::setMaxColors(int maxColors)
{
    m_maxColors = qBound(1, maxColors, 256);
}

/* ---- DSATUR vertex selection: pick uncolored with max saturation, then max degree ---- */

int GraphColoring11::selectVertex(const QVector<VertexState>& state,
                                   const QVector<int>& colors) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] >= 0) continue; // already colored
        if (state[v].saturation > bestSat ||
            (state[v].saturation == bestSat && state[v].degree > bestDeg)) {
            bestSat = state[v].saturation;
            bestDeg = state[v].degree;
            best = v;
        }
    }
    return best;
}

/* ---- Update saturation degrees after coloring a vertex ---- */

void GraphColoring11::propagateColor(int vertex, int color, QVector<VertexState>& state,
                                      const QVector<int>& colors)
{
    for (int nb : m_adjList[vertex]) {
        if (colors[nb] >= 0) continue;
        if (!state[nb].usedColors[color]) {
            state[nb].usedColors[color] = true;
            state[nb].saturation++;
        }
        state[nb].degree--;
    }
}

/* ---- Backtracking search with forward propagation ---- */

bool GraphColoring11::backtrack(QVector<int>& colors, QVector<VertexState>& state,
                                 int colored, int targetColors, int& backtracks)
{
    if (colored == m_numVertices)
        return true;

    int v = selectVertex(state, colors);
    if (v < 0) return true;

    // Try each available color
    for (int c = 0; c < targetColors; ++c) {
        // Check if color is available (not used by neighbors)
        bool available = true;
        for (int nb : m_adjList[v]) {
            if (colors[nb] == c) { available = false; break; }
        }
        if (!available) continue;

        // Assign color
        colors[v] = c;

        // Save neighbor state for rollback
        QVector<int> affectedNeighbors;
        for (int nb : m_adjList[v]) {
            if (colors[nb] < 0) affectedNeighbors.append(nb);
        }

        // Propagate
        for (int nb : affectedNeighbors) {
            if (!state[nb].usedColors[c]) {
                state[nb].usedColors[c] = true;
                state[nb].saturation++;
            }
            state[nb].degree--;
        }

        // Recurse
        if (backtrack(colors, state, colored + 1, targetColors, backtracks))
            return true;

        // Backtrack: undo
        backtracks++;
        colors[v] = -1;
        for (int nb : affectedNeighbors) {
            state[nb].degree++;
        }
        // Rebuild saturation from scratch for affected neighbors
        for (int nb : affectedNeighbors) {
            state[nb].usedColors.assign(targetColors, false);
            state[nb].saturation = 0;
            for (int nb2 : m_adjList[nb]) {
                if (colors[nb2] >= 0 && colors[nb2] < targetColors) {
                    if (!state[nb].usedColors[colors[nb2]]) {
                        state[nb].usedColors[colors[nb2]] = true;
                        state[nb].saturation++;
                    }
                }
            }
        }
    }

    return false;
}

/* ---- Main coloring entry point ---- */

QVector<int> GraphColoring11::color()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0) return {};

    // Initialize vertex states
    QVector<VertexState> state(m_numVertices);
    QVector<int> colors(m_numVertices, -1);

    for (int v = 0; v < m_numVertices; ++v) {
        state[v].degree = m_adjList[v].size();
    }

    // Binary search for chromatic number using DSATUR + backtracking
    int lo = 1, hi = qMin(m_maxColors, m_numVertices);
    int bestChi = hi;
    QVector<int> bestColors;

    // Try from lower bound upward
    for (int k = lo; k <= hi; ++k) {
        // Reset state
        QVector<int> tryColors(m_numVertices, -1);
        QVector<VertexState> tryState(m_numVertices);
        for (int v = 0; v < m_numVertices; ++v) {
            tryState[v].degree = m_adjList[v].size();
            tryState[v].usedColors.assign(k, false);
        }

        int backtracks = 0;
        if (backtrack(tryColors, tryState, 0, k, backtracks)) {
            bestChi = k;
            bestColors = tryColors;
            break;
        }
    }

    if (bestColors.isEmpty()) {
        // Fallback: greedy DSATUR without backtracking
        bestColors.fill(-1);
        bestChi = 0;
        state.assign(m_numVertices, VertexState{});
        for (int v = 0; v < m_numVertices; ++v) {
            state[v].degree = m_adjList[v].size();
            state[v].usedColors.assign(m_maxColors, false);
        }

        for (int step = 0; step < m_numVertices; ++step) {
            int v = selectVertex(state, bestColors);
            if (v < 0) break;
            for (int c = 0; c < m_maxColors; ++c) {
                bool ok = true;
                for (int nb : m_adjList[v]) {
                    if (bestColors[nb] == c) { ok = false; break; }
                }
                if (ok) {
                    bestColors[v] = c;
                    bestChi = qMax(bestChi, c + 1);
                    propagateColor(v, c, state, bestColors);
                    break;
                }
            }
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = 0;
    for (const auto& adj : m_adjList) m_stats.numEdges += adj.size();
    m_stats.numEdges /= 2;
    m_stats.chromaticNumber = bestChi;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringDone(m_numVertices, bestChi, m_stats.backtracks, elapsed);

    return bestColors;
}

/* ---- Accessors ---- */

int GraphColoring11::chromaticNumber() const { return m_stats.chromaticNumber; }

bool GraphColoring11::isValidColoring(const QVector<int>& colors) const
{
    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] < 0) return false;
        for (int nb : m_adjList[v]) {
            if (colors[v] == colors[nb]) return false;
        }
    }
    return true;
}

/* ---- Reset ---- */

void GraphColoring11::resetStatistics()
{
    m_adjList.clear();
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
