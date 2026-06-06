/**
 * @file GraphColoring5.cpp
 * @brief GraphColoring5 实现
 *
 * 实现图着色：DSATUR启发式、回溯精确求解、合法性验证。
 */

#include "utils/graph185/GraphColoring5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphColoring5::GraphColoring5(QObject *parent)
    : QObject(parent)
{
}

GraphColoring5::~GraphColoring5() = default;

/* ---- Configuration ---- */

void GraphColoring5::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_numVertices = adjacency.size();
}

/* ---- Saturation degree ---- */

int GraphColoring5::saturation(int vertex, const QVector<int>& colors) const
{
    QVector<int> usedColors;
    for (int neighbor : m_adj[vertex]) {
        if (colors[neighbor] >= 0 && !usedColors.contains(colors[neighbor]))
            usedColors.append(colors[neighbor]);
    }
    return usedColors.size();
}

int GraphColoring5::selectDSATURVertex(const QVector<int>& colors,
                                         const QVector<bool>& colored) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < m_numVertices; ++v) {
        if (colored[v]) continue;
        int sat = saturation(v, colors);
        int deg = m_adj[v].size();
        if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
            bestSat = sat;
            bestDeg = deg;
            best = v;
        }
    }
    return best;
}

/* ---- DSATUR solver ---- */

GraphColoring5::Result GraphColoring5::solveDSATUR()
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    result.colors.fill(-1, m_numVertices);
    result.isExact = false;
    result.backtracks = 0;

    if (m_numVertices == 0) {
        result.numColors = 0;
        return result;
    }

    QVector<bool> colored(m_numVertices, false);
    int maxColor = -1;

    for (int step = 0; step < m_numVertices; ++step) {
        int v = selectDSATURVertex(result.colors, colored);
        if (v < 0) break;

        /* Find smallest available color */
        QVector<bool> used(m_numVertices, false);
        for (int neighbor : m_adj[v])
            if (result.colors[neighbor] >= 0)
                used[result.colors[neighbor]] = true;

        int color = 0;
        while (color < m_numVertices && used[color]) color++;

        result.colors[v] = color;
        colored[v] = true;
        if (color > maxColor) maxColor = color;
    }

    result.numColors = maxColor + 1;

    m_stats.totalSolves++;
    m_stats.lastColors = result.numColors;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(result.numColors, false);
    return result;
}

/* ---- Backtracking helpers ---- */

bool GraphColoring5::canColor(int vertex, int color, const QVector<int>& colors) const
{
    for (int neighbor : m_adj[vertex])
        if (colors[neighbor] == color) return false;
    return true;
}

int GraphColoring5::selectVertex(const QVector<int>& colors) const
{
    /* MRV: minimum remaining values (uncolored with most constraints) */
    int best = -1;
    int bestConstraint = -1;
    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] >= 0) continue;
        int constraints = 0;
        for (int nb : m_adj[v])
            if (colors[nb] >= 0) constraints++;
        if (constraints > bestConstraint) {
            bestConstraint = constraints;
            best = v;
        }
    }
    return best;
}

bool GraphColoring5::backtrack(QVector<int>& colors, int vertex, int maxColors,
                                Result& best, int numColored)
{
    if (numColored == m_numVertices) {
        /* All vertices colored */
        int used = 0;
        for (int c : colors) if (c + 1 > used) used = c + 1;
        if (used < best.numColors || best.numColors == 0) {
            best.colors = colors;
            best.numColors = used;
            best.backtracks = 0;
        }
        return true;
    }

    if (vertex < 0) return false;

    for (int c = 0; c < maxColors; ++c) {
        if (canColor(vertex, c, colors)) {
            colors[vertex] = c;
            int next = selectVertex(colors);
            if (backtrack(colors, next, maxColors, best, numColored + 1))
                return true;
            colors[vertex] = -1;
            best.backtracks++;
        }
    }
    return false;
}

/* ---- Exact solver ---- */

GraphColoring5::Result GraphColoring5::solveExact(int upperBound)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    result.isExact = true;
    result.backtracks = 0;

    if (m_numVertices == 0) {
        result.numColors = 0;
        return result;
    }

    /* Use DSATUR as upper bound if not provided */
    if (upperBound <= 0)
        upperBound = solveDSATUR().numColors;

    /* Binary search for chromatic number */
    int lo = 1, hi = upperBound;
    result.numColors = upperBound;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        QVector<int> colors(m_numVertices, -1);
        Result trial;
        trial.numColors = upperBound + 1;
        trial.backtracks = 0;
        trial.isExact = true;

        int start = selectVertex(colors);
        bool ok = backtrack(colors, start, mid, trial, 0);

        if (ok && trial.numColors <= mid) {
            result = trial;
            result.numColors = trial.numColors;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }

    m_stats.totalSolves++;
    m_stats.lastColors = result.numColors;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(result.numColors, true);
    return result;
}

/* ---- Validation ---- */

bool GraphColoring5::validate(const QVector<int>& colors) const
{
    if (colors.size() != m_numVertices) return false;
    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] < 0) return false;
        for (int nb : m_adj[v])
            if (colors[v] == colors[nb]) return false;
    }
    return true;
}

/* ---- Statistics ---- */

void GraphColoring5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
