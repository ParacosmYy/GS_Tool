/**
 * @file VertexCover10.cpp
 * @brief VertexCover10 实现
 *
 * 实现顶点覆盖：LP松弛舍入与最大匹配2近似有界预算优化。
 */

#include "utils/graph285/VertexCover10.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

VertexCover10::VertexCover10(QObject *parent)
    : QObject(parent) {}

VertexCover10::~VertexCover10() = default;

/* ---- Configuration ---- */

void VertexCover10::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_n = adjacency.size();
}

void VertexCover10::setBudget(int budget)
{
    m_budget = qBound(0, budget, 100000);
}

/* ---- Count edges in graph ---- */

static int countEdges(const QVector<QVector<int>>& adj, int n)
{
    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (i < adj[i].size() && adj[i][j])
                ++edges;
    return edges;
}

/* ---- Maximal matching 2-approximation ---- */

QVector<int> VertexCover10::solveMatching()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<bool> inCover(m_n, false);
    QVector<bool> matched(m_n, false);

    // Greedy maximal matching: pick uncovered edges, add both endpoints
    for (int i = 0; i < m_n; ++i) {
        if (matched[i]) continue;
        if (i >= m_adj.size()) continue;
        for (int j = i + 1; j < m_n; ++j) {
            if (matched[j]) continue;
            if (j >= m_adj[i].size()) continue;
            if (m_adj[i][j]) {
                // Found an uncovered edge — add both endpoints
                inCover[i] = true;
                inCover[j] = true;
                matched[i] = true;
                matched[j] = true;
                break; // move to next unmatched vertex
            }
        }
    }

    QVector<int> cover;
    for (int i = 0; i < m_n; ++i)
        if (inCover[i]) cover.append(i);

    int edges = countEdges(m_adj, m_n);
    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges;
    m_stats.coverSize = cover.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coverFound(cover.size(), m_n, edges, elapsed);

    return cover;
}

/* ---- LP relaxation solver (simple iterative method) ---- */

QVector<double> VertexCover10::solveLP() const
{
    // LP: min sum(x_i), s.t. x_i + x_j >= 1 for each edge (i,j), 0 <= x_i <= 1
    // Simple iterative rounding via dual: start with x_i = 0.5 for all
    int n = m_n;
    QVector<double> x(n, 0.5);

    // Iterative constraint satisfaction via coordinate descent
    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            if (i >= m_adj.size()) continue;
            for (int j = i + 1; j < n; ++j) {
                if (j >= m_adj[i].size()) continue;
                if (!m_adj[i][j]) continue;
                double sum = x[i] + x[j];
                if (sum < 1.0) {
                    // Distribute deficit equally
                    double deficit = (1.0 - sum) / 2.0;
                    x[i] = qMin(1.0, x[i] + deficit);
                    x[j] = qMin(1.0, x[j] + deficit);
                    changed = true;
                }
            }
        }
        if (!changed) break;
    }
    return x;
}

/* ---- LP relaxation rounding ---- */

QVector<int> VertexCover10::solveLPRounding()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<double> x = solveLP();

    // Round: include vertex if x_i >= 0.5
    QVector<int> cover;
    for (int i = 0; i < m_n; ++i)
        if (x[i] >= 0.5) cover.append(i);

    // Verify cover and add any missing vertices
    for (int i = 0; i < m_n; ++i) {
        if (i >= m_adj.size()) continue;
        for (int j = i + 1; j < m_n; ++j) {
            if (j >= m_adj[i].size()) continue;
            if (m_adj[i][j]) {
                bool covered = false;
                for (int v : cover)
                    if (v == i || v == j) { covered = true; break; }
                if (!covered) cover.append(i);
            }
        }
    }

    int edges = countEdges(m_adj, m_n);
    double lpObj = 0.0;
    for (double v : x) lpObj += v;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges;
    m_stats.coverSize = cover.size();
    m_stats.lpObjective = lpObj;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coverFound(cover.size(), m_n, edges, elapsed);

    return cover;
}

/* ---- Budgeted vertex cover ---- */

QVector<int> VertexCover10::solveBudgeted()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    QVector<double> x = solveLP();

    // Sort vertices by fractional value descending (greedy)
    QVector<QPair<double, int>> sorted;
    for (int i = 0; i < m_n; ++i)
        sorted.append({x[i], i});
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    int budget = (m_budget > 0) ? qMin(m_budget, m_n) : m_n;
    QVector<int> cover;

    // Select top-B vertices by LP value
    for (int i = 0; i < budget && i < sorted.size(); ++i)
        cover.append(sorted[i].second);

    int edges = countEdges(m_adj, m_n);
    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges;
    m_stats.coverSize = cover.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coverFound(cover.size(), m_n, edges, elapsed);

    return cover;
}

/* ---- Validate cover ---- */

bool VertexCover10::validateCover(const QVector<int>& cover) const
{
    QSet<int> coverSet;
    for (int v : cover) coverSet.insert(v);

    for (int i = 0; i < m_n; ++i) {
        if (i >= m_adj.size()) continue;
        for (int j = i + 1; j < m_n; ++j) {
            if (j >= m_adj[i].size()) continue;
            if (m_adj[i][j] && !coverSet.contains(i) && !coverSet.contains(j))
                return false;
        }
    }
    return true;
}

/* ---- Accessor ---- */

QVector<QVector<int>> VertexCover10::graph() const { return m_adj; }

/* ---- Reset ---- */

void VertexCover10::resetStatistics()
{
    m_adj.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
