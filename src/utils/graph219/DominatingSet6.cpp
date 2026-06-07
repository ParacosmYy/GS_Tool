/**
 * @file DominatingSet6.cpp
 * @brief DominatingSet6 实现
 *
 * 实现LP舍入支配集：LP松弛求解、随机化舍入、迭代精炼去除冗余顶点。
 */

#include "utils/graph219/DominatingSet6.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DominatingSet6::DominatingSet6(QObject *parent) : QObject(parent) {}
DominatingSet6::~DominatingSet6() = default;

/* ---- Configuration ---- */

void DominatingSet6::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void DominatingSet6::setRoundTrials(int trials) { m_roundTrials = qMax(1, trials); }

/* ---- Count uncovered ---- */

int DominatingSet6::countUncovered(const QVector<bool>& covered) const
{
    int count = 0;
    for (bool c : covered)
        if (!c) count++;
    return count;
}

/* ---- Solve LP relaxation ---- */

QVector<double> DominatingSet6::solveLP(const QVector<QVector<int>>& adjList) const
{
    int n = adjList.size();
    QVector<double> x(n, 0.0);

    // Greedy LP approximation: set x[v] proportional to inverse degree coverage
    // For each vertex v, x[v] >= 1 / (1 + deg(v)) to cover itself
    // Iterative: find uncovered vertex with minimum "cost per coverage"

    QVector<bool> covered(n, false);
    QVector<double> residual(n, 1.0);  // Each vertex needs total coverage of 1.0

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Find most uncovered vertex
        int best = -1;
        double bestNeed = 0.0;
        for (int v = 0; v < n; ++v) {
            if (covered[v]) continue;
            if (residual[v] > bestNeed) { bestNeed = residual[v]; best = v; }
        }
        if (best < 0) break;

        // Distribute coverage to best and its neighbors
        double share = 1.0 / (1 + adjList[best].size());
        double alloc = qMin(share, residual[best]);
        x[best] += alloc;
        residual[best] -= alloc;
        if (residual[best] <= 1e-10) covered[best] = true;

        for (int nb : adjList[best]) {
            if (nb < 0 || nb >= n) continue;
            double a = qMin(share, residual[nb]);
            residual[nb] -= a;
            if (residual[nb] <= 1e-10) covered[nb] = true;
        }
    }

    // Clamp to [0, 1]
    for (int i = 0; i < n; ++i)
        x[i] = qBound(0.0, x[i], 1.0);

    return x;
}

/* ---- Randomized rounding ---- */

QVector<int> DominatingSet6::randomizedRound(const QVector<double>& fractional,
                                              const QVector<QVector<int>>& adjList) const
{
    int n = fractional.size();
    QVector<int> selected;
    QVector<bool> covered(n, false);

    // Round each vertex independently with probability x[v]
    // Repeat until all vertices are covered
    for (int trial = 0; trial < m_roundTrials; ++trial) {
        selected.clear();
        covered.fill(false);

        for (int v = 0; v < n; ++v) {
            double prob = fractional[v];
            // Boost probability for uncovered vertices
            if (!covered[v]) prob = qMin(1.0, prob * 2.0);

            double r = QRandomGenerator::global()->generateDouble();
            if (r < prob) {
                selected.append(v);
                covered[v] = true;
                for (int nb : adjList[v])
                    if (nb >= 0 && nb < n) covered[nb] = true;
            }
        }

        if (countUncovered(covered) == 0) break;
    }

    return selected;
}

/* ---- Iterative refinement ---- */

QVector<int> DominatingSet6::refine(const QVector<int>& dominating,
                                     const QVector<QVector<int>>& adjList) const
{
    int n = adjList.size();
    QVector<bool> inSet(n, false);
    for (int v : dominating)
        if (v >= 0 && v < n) inSet[v] = true;

    // Try removing each vertex; if still dominating, remove it
    QVector<int> refined = dominating;
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = refined.size() - 1; i >= 0; --i) {
            int v = refined[i];
            inSet[v] = false;

            // Check if neighbors are still covered
            bool stillValid = true;
            // Check v itself
            bool vCovered = false;
            for (int nb : adjList[v])
                if (nb >= 0 && nb < n && inSet[nb]) { vCovered = true; break; }
            if (!vCovered) stillValid = false;

            // Check all neighbors of v
            if (stillValid) {
                for (int nb : adjList[v]) {
                    if (nb < 0 || nb >= n || inSet[nb]) continue;
                    // Neighbor nb is not in set; check if covered by others
                    bool nbCovered = false;
                    for (int nb2 : adjList[nb])
                        if (nb2 >= 0 && nb2 < n && inSet[nb2]) { nbCovered = true; break; }
                    if (!nbCovered) { stillValid = false; break; }
                }
            }

            if (stillValid) {
                refined.removeAt(i);
                changed = true;
            } else {
                inSet[v] = true;
            }
        }
    }

    return refined;
}

/* ---- Check domination ---- */

bool DominatingSet6::isDominating(const QVector<int>& vertices,
                                   const QVector<QVector<int>>& adjList) const
{
    int n = adjList.size();
    QVector<bool> covered(n, false);
    for (int v : vertices) {
        if (v < 0 || v >= n) continue;
        covered[v] = true;
        for (int nb : adjList[v])
            if (nb >= 0 && nb < n) covered[nb] = true;
    }
    for (bool c : covered)
        if (!c) return false;
    return true;
}

/* ---- Solve ---- */

QVector<int> DominatingSet6::solve(const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();
    int n = adjList.size();
    if (n == 0) return {};

    // Step 1: Solve LP relaxation
    QVector<double> fractional = solveLP(adjList);

    // Step 2: Randomized rounding
    QVector<int> domSet = randomizedRound(fractional, adjList);

    // Step 3: Iterative refinement
    domSet = refine(domSet, adjList);

    m_stats.totalSolves++;
    m_stats.numVertices = n;
    m_stats.dominatingSetSize = domSet.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(domSet.size(), timer.elapsed());
    return domSet;
}

/* ---- Reset ---- */

void DominatingSet6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
