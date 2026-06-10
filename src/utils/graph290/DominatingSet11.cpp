/**
 * @file DominatingSet11.cpp
 * @brief DominatingSet11 实现
 *
 * 实现支配集：贪心随机自适应搜索与局部搜索改进的元启发式优化。
 */

#include "utils/graph290/DominatingSet11.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DominatingSet11::DominatingSet11(QObject *parent)
    : QObject(parent) {}

DominatingSet11::~DominatingSet11() = default;

/* ---- Configuration ---- */

void DominatingSet11::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_stats.numVertices = adjacency.size();
    m_stats.numEdges = 0;
    for (const auto& neighbors : adjacency) m_stats.numEdges += neighbors.size();
    m_stats.numEdges /= 2;
}

void DominatingSet11::setWeights(const QVector<double>& weights)
{
    m_weights = weights;
}

void DominatingSet11::setGRASPParams(int maxIterations, double alpha)
{
    m_maxIter = qBound(1, maxIterations, 10000);
    m_alpha = qBound(0.0, alpha, 1.0);
}

/* ---- Check if vertex v is dominated ---- */

bool DominatingSet11::isDominated(int v, const QSet<int>& domSet) const
{
    if (domSet.contains(v)) return true;
    if (v < 0 || v >= m_adj.size()) return false;
    for (int u : m_adj[v]) {
        if (domSet.contains(u)) return true;
    }
    return false;
}

/* ---- Count undominated vertices ---- */

int DominatingSet11::countUndominated(const QSet<int>& domSet) const
{
    int count = 0;
    for (int v = 0; v < m_adj.size(); ++v) {
        if (!isDominated(v, domSet)) count++;
    }
    return count;
}

/* ---- Coverage gain of adding vertex v ---- */

int DominatingSet11::coverageGain(int v, const QSet<int>& domSet) const
{
    int gain = 0;
    if (!isDominated(v, domSet)) gain++;  // v itself
    if (v >= 0 && v < m_adj.size()) {
        for (int u : m_adj[v]) {
            if (!isDominated(u, domSet)) gain++;
        }
    }
    return gain;
}

/* ---- Compute weighted cost ---- */

double DominatingSet11::computeCost(const QVector<int>& vertices) const
{
    if (m_weights.isEmpty()) return static_cast<double>(vertices.size());
    double cost = 0.0;
    for (int v : vertices) {
        if (v >= 0 && v < m_weights.size()) cost += m_weights[v];
        else cost += 1.0;
    }
    return cost;
}

/* ---- Construct greedy randomized solution ---- */

DominatingSet11::Solution DominatingSet11::constructGreedyRandomized()
{
    int n = m_adj.size();
    Solution sol;
    QSet<int> domSet;

    while (countUndominated(domSet) > 0) {
        // Compute coverage gains for all vertices
        QVector<QPair<int, int>> gains;  // (gain, vertex)
        for (int v = 0; v < n; ++v) {
            int g = coverageGain(v, domSet);
            if (g > 0) gains.append({g, v});
        }
        if (gains.isEmpty()) break;

        // Sort by gain (descending)
        std::sort(gains.begin(), gains.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // Build Restricted Candidate List (RCL)
        int maxGain = gains[0].first;
        int minGain = gains.last().first;
        double threshold = maxGain - m_alpha * (maxGain - minGain);
        QVector<int> rcl;
        for (const auto& p : gains) {
            if (p.first >= threshold) rcl.append(p.second);
        }

        // Randomly select from RCL
        int selected = rcl[qrand() % rcl.size()];
        domSet.insert(selected);
        sol.vertices.append(selected);
    }

    sol.size = sol.vertices.size();
    sol.cost = computeCost(sol.vertices);
    sol.feasible = (countUndominated(domSet) == 0);
    return sol;
}

/* ---- Local search improvement ---- */

DominatingSet11::Solution DominatingSet11::localSearch(const Solution& sol)
{
    Solution best = sol;
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < best.vertices.size(); ++i) {
            // Try removing vertex i
            QSet<int> testSet;
            for (int j = 0; j < best.vertices.size(); ++j) {
                if (j != i) testSet.insert(best.vertices[j]);
            }

            if (countUndominated(testSet) == 0) {
                // Removal is feasible — improvement found
                Solution newSol;
                for (int v : testSet) newSol.vertices.append(v);
                newSol.size = newSol.vertices.size();
                newSol.cost = computeCost(newSol.vertices);
                newSol.feasible = true;
                best = newSol;
                improved = true;
                break;  // Restart local search
            }

            // Try swap: replace vertex i with non-member vertex
            for (int v = 0; v < m_adj.size(); ++v) {
                if (testSet.contains(v)) continue;
                QSet<int> swapSet = testSet;
                swapSet.insert(v);
                if (countUndominated(swapSet) == 0) {
                    double swapCost = computeCost(newSol.vertices);
                    // Check if weighted improvement
                    double oldCost = (i < m_weights.size()) ? m_weights[best.vertices[i]] : 1.0;
                    double newCost = (v < m_weights.size()) ? m_weights[v] : 1.0;
                    if (newCost < oldCost) {
                        Solution swapSol;
                        for (int s : swapSet) swapSol.vertices.append(s);
                        swapSol.size = swapSol.vertices.size();
                        swapSol.cost = computeCost(swapSol.vertices);
                        swapSol.feasible = true;
                        if (swapSol.cost < best.cost) {
                            best = swapSol;
                            improved = true;
                            break;
                        }
                    }
                }
            }
            if (improved) break;
        }
    }

    return best;
}

/* ---- Solve with GRASP ---- */

DominatingSet11::Solution DominatingSet11::solve()
{
    QElapsedTimer timer;
    timer.start();

    Solution best;
    best.size = m_adj.size() + 1;
    best.cost = 1e18;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        Solution candidate = constructGreedyRandomized();
        candidate = localSearch(candidate);

        if (candidate.feasible && candidate.cost < best.cost) {
            best = candidate;
            emit solutionFound(best.size, best.cost, iter, timer.elapsed());
        }
    }

    if (best.cost >= 1e18) best.feasible = false;

    double elapsed = timer.elapsed();
    m_stats.bestSize = best.size;
    m_stats.numIterations = m_maxIter;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return best;
}

/* ---- Solve with pure greedy ---- */

DominatingSet11::Solution DominatingSet11::solveGreedy()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_adj.size();
    Solution sol;
    QSet<int> domSet;
    QVector<bool> inSet(n, false);

    while (countUndominated(domSet) > 0) {
        int bestV = -1;
        int bestGain = -1;
        double bestWeight = 1e18;

        for (int v = 0; v < n; ++v) {
            if (inSet[v]) continue;
            int g = coverageGain(v, domSet);
            double w = (v < m_weights.size()) ? m_weights[v] : 1.0;
            // Prefer higher gain per unit weight
            double score = (w > 0) ? g / w : g;
            double bestScore = (bestWeight > 0) ? bestGain / bestWeight : bestGain;
            if (g > bestGain || (g == bestGain && w < bestWeight)) {
                bestV = v;
                bestGain = g;
                bestWeight = w;
            }
        }

        if (bestV < 0) break;
        domSet.insert(bestV);
        inSet[bestV] = true;
        sol.vertices.append(bestV);
    }

    sol.size = sol.vertices.size();
    sol.cost = computeCost(sol.vertices);
    sol.feasible = (countUndominated(domSet) == 0);

    double elapsed = timer.elapsed();
    m_stats.bestSize = sol.size;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return sol;
}

/* ---- Verify candidate set ---- */

bool DominatingSet11::verify(const QVector<int>& candidateSet) const
{
    QSet<int> domSet;
    for (int v : candidateSet) domSet.insert(v);
    return countUndominated(domSet) == 0;
}

/* ---- Get graph ---- */

QVector<QVector<int>> DominatingSet11::graph() const { return m_adj; }

/* ---- Reset ---- */

void DominatingSet11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
