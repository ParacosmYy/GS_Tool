/**
 * @file IndependentSet4.cpp
 * @brief IndependentSet4 实现
 *
 * 实现最大权独立集：分支约减、LP松弛界、顶点折叠。
 */

#include "utils/graph205/IndependentSet4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

IndependentSet4::IndependentSet4(QObject *parent) : QObject(parent) {}
IndependentSet4::~IndependentSet4() = default;

/* ---- Configuration ---- */

void IndependentSet4::setGraph(const QVector<QVector<int>>& adj,
                                const QVector<double>& weights)
{
    m_adj = adj;
    m_weights = weights;
}

/* ---- Check independence ---- */

bool IndependentSet4::isIndependent(int v,
                                     const QVector<int>& current) const
{
    for (int u : current) {
        // Check if v and u are adjacent
        for (int nb : m_adj[v]) {
            if (nb == u) return false;
        }
    }
    return true;
}

/* ---- LP relaxation bound ---- */

double IndependentSet4::lpBound(const QVector<int>& candidates,
                                 const QVector<bool>& excluded) const
{
    // Simple LP relaxation: sum of max(weights) per connected component
    // Upper bound is sum of all candidate weights (trivially feasible)
    // Tighter: for each vertex, it competes with its neighbors
    double bound = 0.0;
    for (int v : candidates) {
        if (excluded[v]) continue;
        // Vertex contributes min(weight[v], sum of neighbor weights / max(1, degree))
        double neighborSum = 0.0;
        int degree = 0;
        for (int nb : m_adj[v]) {
            if (!excluded[nb]) {
                neighborSum += m_weights[nb];
                degree++;
            }
        }
        if (degree == 0) {
            bound += m_weights[v];
        } else {
            // Fractional relaxation: each edge constrains at most one endpoint
            bound += qMin(m_weights[v], neighborSum / degree);
        }
    }
    return bound;
}

/* ---- Degree-1 vertex folding ---- */

void IndependentSet4::foldDegree1(QVector<int>& candidates,
                                   QVector<bool>& excluded,
                                   QVector<int>& folded)
{
    for (int v : candidates) {
        if (excluded[v]) continue;
        int degree = 0;
        int neighbor = -1;
        for (int nb : m_adj[v]) {
            if (!excluded[nb]) {
                degree++;
                neighbor = nb;
                if (degree > 1) break;
            }
        }
        if (degree == 1 && neighbor >= 0) {
            // Fold: include v (weight gain), exclude neighbor
            double gainW = m_weights[v];
            double lossW = m_weights[neighbor];

            if (gainW >= lossW) {
                // Always include v, exclude neighbor
                folded.append(v);
                excluded[neighbor] = true;
            } else {
                // Fold: either include v or include neighbor
                folded.append(v);
                m_weights[neighbor] -= m_weights[v];
            }
            m_stats.foldsApplied++;
        }
    }
}

/* ---- Dominated vertex removal ---- */

void IndependentSet4::removeDominated(QVector<int>& candidates,
                                       QVector<bool>& excluded)
{
    for (int v : candidates) {
        if (excluded[v]) continue;
        for (int u : m_adj[v]) {
            if (excluded[u]) continue;
            // u dominates v if N[v] subset of N[u] and weight[u] >= weight[v]
            bool dominated = (m_weights[u] >= m_weights[v]);
            if (!dominated) continue;

            QVector<int> nvNeighbors, nuNeighbors;
            for (int nb : m_adj[v])
                if (!excluded[nb]) nvNeighbors.append(nb);
            for (int nb : m_adj[u])
                if (!excluded[nb]) nuNeighbors.append(nb);

            // Check if all v's closed neighborhood is in u's closed neighborhood
            for (int nb : nvNeighbors) {
                bool found = (nb == u);
                for (int unb : nuNeighbors) {
                    if (unb == nb) { found = true; break; }
                }
                if (!found) { dominated = false; break; }
            }

            if (dominated) {
                excluded[v] = true;
                break;
            }
        }
    }
}

/* ---- Apply reductions ---- */

bool IndependentSet4::applyFolding(QVector<int>& candidates,
                                    QVector<bool>& excluded,
                                    QVector<int>& folded)
{
    int before = m_stats.foldsApplied;
    foldDegree1(candidates, excluded, folded);
    removeDominated(candidates, excluded);
    return m_stats.foldsApplied > before;
}

/* ---- Branch-and-reduce ---- */

void IndependentSet4::branchAndReduce(QVector<int>& candidates,
                                       QVector<int>& current,
                                       double currentWeight,
                                       QVector<bool>& excluded)
{
    m_stats.branchesExplored++;

    // Pruning: LP bound check
    double bound = currentWeight + lpBound(candidates, excluded);
    if (bound <= m_bestWeight) return;

    // Apply reductions
    QVector<int> folded;
    applyFolding(candidates, excluded, folded);

    // Find branching vertex: highest degree candidate
    int branchV = -1;
    int maxDeg = -1;
    for (int v : candidates) {
        if (excluded[v]) continue;
        int deg = 0;
        for (int nb : m_adj[v])
            if (!excluded[nb]) deg++;
        if (deg > maxDeg) {
            maxDeg = deg;
            branchV = v;
        }
    }

    if (branchV < 0) {
        // All candidates exhausted
        if (currentWeight > m_bestWeight) {
            m_bestWeight = currentWeight;
            m_bestSet = current + folded;
        }
        return;
    }

    // Branch 1: Include branchV
    QVector<bool> exc1 = excluded;
    QVector<int> cur1 = current;
    cur1.append(branchV);
    // Exclude neighbors
    for (int nb : m_adj[branchV])
        exc1[nb] = true;

    branchAndReduce(candidates, cur1, currentWeight + m_weights[branchV], exc1);

    // Branch 2: Exclude branchV
    QVector<bool> exc2 = excluded;
    exc2[branchV] = true;

    branchAndReduce(candidates, current, currentWeight, exc2);
}

/* ---- Solve ---- */

QVector<int> IndependentSet4::solve()
{
    QElapsedTimer timer;
    timer.start();

    int N = m_adj.size();
    if (N == 0) return {};

    m_bestWeight = 0.0;
    m_bestSet.clear();
    m_stats.branchesExplored = 0;
    m_stats.foldsApplied = 0;

    QVector<int> candidates;
    candidates.reserve(N);
    for (int i = 0; i < N; ++i) candidates.append(i);

    QVector<int> current;
    QVector<bool> excluded(N, false);

    branchAndReduce(candidates, current, 0.0, excluded);

    // Count edges
    int edgeCount = 0;
    for (int i = 0; i < N; ++i)
        edgeCount += m_adj[i].size();
    edgeCount /= 2;

    m_stats.totalSolves++;
    m_stats.numVertices = N;
    m_stats.numEdges = edgeCount;
    m_stats.setSize = m_bestSet.size();
    m_stats.totalWeight = m_bestWeight;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_bestSet.size(), m_bestWeight, timer.elapsed());
    return m_bestSet;
}

/* ---- Best weight accessor ---- */

double IndependentSet4::bestWeight() const { return m_bestWeight; }

/* ---- Reset ---- */

void IndependentSet4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bestSet.clear();
    m_bestWeight = 0.0;
}
