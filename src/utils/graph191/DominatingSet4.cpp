/**
 * @file DominatingSet4.cpp
 * @brief DominatingSet4 实现
 *
 * 实现最小支配集：贪心近似、迭代改进、顶点覆盖评分。
 */

#include "utils/graph191/DominatingSet4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DominatingSet4::DominatingSet4(QObject *parent)
    : QObject(parent)
{
}

DominatingSet4::~DominatingSet4() = default;

/* ---- Configuration ---- */

void DominatingSet4::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_numVertices = adjacency.size();
}

/* ---- Vertex coverage score ---- */

double DominatingSet4::vertexCoverageScore(int vertex) const
{
    if (vertex < 0 || vertex >= m_numVertices) return 0.0;
    /* Score = number of uncovered neighbors + 1 (self) */
    /* Relative to degree */
    int degree = m_adj[vertex].size();
    return (degree + 1.0) / qMax(1, m_numVertices);
}

QVector<QPair<int, double>> DominatingSet4::allCoverageScores() const
{
    QVector<QPair<int, double>> scores;
    for (int v = 0; v < m_numVertices; ++v)
        scores.append({v, vertexCoverageScore(v)});
    std::sort(scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    return scores;
}

/* ---- Main solver ---- */

QVector<int> DominatingSet4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0) return {};

    /* Phase 1: Greedy construction */
    QVector<bool> dominated(m_numVertices, false);
    QVector<int> domSet;

    /* Each vertex dominates itself and its neighbors */
    while (true) {
        /* Find undominated vertex with highest coverage */
        int bestV = -1;
        int bestNewCoverage = -1;

        for (int v = 0; v < m_numVertices; ++v) {
            if (dominated[v]) continue; /* Skip already-dominated */

            /* Count how many new vertices v would cover */
            int newCov = dominated[v] ? 0 : 1; /* Self */
            for (int nb : m_adj[v]) {
                if (!dominated[nb]) newCov++;
            }

            if (newCov > bestNewCoverage) {
                bestNewCoverage = newCov;
                bestV = v;
            }
        }

        if (bestV < 0 || bestNewCoverage <= 0) break;

        /* Add bestV to dominating set */
        domSet.append(bestV);
        dominated[bestV] = true;
        for (int nb : m_adj[bestV])
            dominated[nb] = true;
    }

    /* Phase 2: Iterative improvement - try removing redundant vertices */
    bool improved = true;
    int iteration = 0;
    while (improved) {
        improved = false;
        iteration++;

        for (int i = domSet.size() - 1; i >= 0; --i) {
            int v = domSet[i];

            /* Check if v is redundant: temporarily remove v and verify */
            QVector<bool> covered(m_numVertices, false);
            for (int j = 0; j < domSet.size(); ++j) {
                if (j == i) continue;
                covered[domSet[j]] = true;
                for (int nb : m_adj[domSet[j]])
                    covered[nb] = true;
            }

            bool allCovered = true;
            for (int k = 0; k < m_numVertices; ++k) {
                if (!covered[k]) { allCovered = false; break; }
            }

            if (allCovered) {
                domSet.removeAt(i);
                improved = true;
                break; /* Restart check */
            }
        }
        emit improvementStep(iteration, domSet.size());
    }

    m_dominatingSet = domSet;

    /* Compute coverage ratio */
    int coveredCount = 0;
    for (int i = 0; i < m_numVertices; ++i) {
        bool cov = false;
        for (int v : domSet) {
            if (v == i) { cov = true; break; }
            for (int nb : m_adj[v]) {
                if (nb == i) { cov = true; break; }
            }
            if (cov) break;
        }
        if (cov) coveredCount++;
    }

    double coverage = m_numVertices > 0 ?
        static_cast<double>(coveredCount) / m_numVertices : 0.0;

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_stats.dominatingsetSize = domSet.size();
    m_stats.coverageRatio = coverage;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit solveCompleted(domSet.size(), coverage);
    return domSet;
}

/* ---- Validation ---- */

bool DominatingSet4::validate(const QVector<int>& dominatingSet) const
{
    QVector<bool> covered(m_numVertices, false);

    for (int v : dominatingSet) {
        if (v < 0 || v >= m_numVertices) return false;
        covered[v] = true;
        for (int nb : m_adj[v])
            covered[nb] = true;
    }

    for (int i = 0; i < m_numVertices; ++i)
        if (!covered[i]) return false;
    return true;
}

void DominatingSet4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
