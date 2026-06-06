/**
 * @file DominatingSet3.cpp
 * @brief DominatingSet3 实现
 *
 * 实现最小支配集贪心近似：覆盖率优先选择、未来覆盖度打破平局。
 */

#include "utils/graph186/DominatingSet3.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DominatingSet3::DominatingSet3(QObject *parent)
    : QObject(parent)
{
}

DominatingSet3::~DominatingSet3() = default;

/* ---- Coverage gain: how many new vertices this vertex would cover ---- */

int DominatingSet3::coverageGain(int vertex,
                                  const QVector<QVector<int>>& adjList,
                                  const QVector<bool>& dominated) const
{
    int gain = dominated[vertex] ? 0 : 1;
    for (int nb : adjList[vertex]) {
        if (!dominated[nb]) gain++;
    }
    return gain;
}

/* ---- Future coverage: uncovered 2-hop neighbors (tie-breaking) ---- */

int DominatingSet3::futureCoverage(int vertex,
                                    const QVector<QVector<int>>& adjList,
                                    const QVector<bool>& dominated) const
{
    int future = 0;
    /* Count uncovered neighbors of neighbors */
    for (int nb : adjList[vertex]) {
        for (int nb2 : adjList[nb]) {
            if (!dominated[nb2]) future++;
        }
    }
    return future;
}

/* ---- Main solver ---- */

QVector<int> DominatingSet3::solve(const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjList.size();
    if (n == 0) return {};

    QVector<bool> dominated(n, false);
    QVector<bool> inSet(n, false);
    QVector<int> domSet;

    int dominatedCount = 0;

    while (dominatedCount < n) {
        int bestVertex = -1;
        int bestGain = 0;
        int bestFuture = -1;

        for (int v = 0; v < n; ++v) {
            if (inSet[v]) continue;

            int gain = coverageGain(v, adjList, dominated);
            if (gain == 0) continue;

            int future = futureCoverage(v, adjList, dominated);

            /* Select vertex with highest gain; break ties by future coverage */
            if (gain > bestGain || (gain == bestGain && future > bestFuture)) {
                bestGain = gain;
                bestFuture = future;
                bestVertex = v;
            }
        }

        if (bestVertex < 0) {
            /* All remaining vertices are isolated and dominated */
            for (int v = 0; v < n; ++v) {
                if (!dominated[v]) {
                    domSet.append(v);
                    dominated[v] = true;
                    dominatedCount++;
                }
            }
            break;
        }

        /* Add best vertex to dominating set */
        domSet.append(bestVertex);
        inSet[bestVertex] = true;

        if (!dominated[bestVertex]) {
            dominated[bestVertex] = true;
            dominatedCount++;
        }

        for (int nb : adjList[bestVertex]) {
            if (!dominated[nb]) {
                dominated[nb] = true;
                dominatedCount++;
            }
        }
    }

    m_dominated = dominated;

    m_stats.totalSolves++;
    m_stats.lastSetSize = domSet.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(domSet.size());
    return domSet;
}

/* ---- Validation ---- */

bool DominatingSet3::validate(const QVector<QVector<int>>& adjList,
                               const QVector<int>& domSet) const
{
    int n = adjList.size();
    QVector<bool> dominated(n, false);

    for (int v : domSet) {
        if (v < 0 || v >= n) return false;
        dominated[v] = true;
        for (int nb : adjList[v])
            dominated[nb] = true;
    }

    for (int i = 0; i < n; ++i)
        if (!dominated[i]) return false;
    return true;
}

/* ---- Coverage map ---- */

QVector<bool> DominatingSet3::coverageMap() const
{
    return m_dominated;
}

/* ---- Statistics ---- */

void DominatingSet3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
