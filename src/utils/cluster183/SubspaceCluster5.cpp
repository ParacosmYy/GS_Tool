/**
 * @file SubspaceCluster5.cpp
 * @brief SubspaceCluster5 实现
 *
 * 实现PROCLUS子空间聚类：投影追求维度选择、曼哈顿段距离、贪心初始化、迭代精化。
 */

#include "utils/cluster183/SubspaceCluster5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SubspaceCluster5::SubspaceCluster5(QObject *parent) : QObject(parent) {}
SubspaceCluster5::~SubspaceCluster5() = default;

/* ---- Configuration ---- */

void SubspaceCluster5::setNumClusters(int k) { m_numClusters = qMax(1, k); }
void SubspaceCluster5::setAvgDimensions(int l) { m_avgDimensions = qMax(1, l); }
void SubspaceCluster5::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }

/* ---- Manhattan segmental distance ---- */

double SubspaceCluster5::manhattanSegmental(const QVector<double>& a,
                                             const QVector<double>& b,
                                             const QVector<int>& dims) const
{
    if (dims.isEmpty()) return 0.0;
    double sum = 0.0;
    for (int d : dims) {
        if (d >= 0 && d < a.size() && d < b.size())
            sum += qAbs(a[d] - b[d]);
    }
    return sum / dims.size();
}

/* ---- Dimension selection via projection pursuit ---- */

QVector<int> SubspaceCluster5::selectDimensions(
    const QVector<QVector<double>>& data,
    int medoidIdx,
    const QVector<int>& candidateIdx) const
{
    int D = data[0].size();
    int k = candidateIdx.size();
    if (k == 0 || D == 0) return {};

    // Compute per-dimension dispersion from medoid to neighbors
    QVector<QPair<double, int>> dimScores(D);
    for (int d = 0; d < D; ++d) {
        double sum = 0.0;
        for (int idx : candidateIdx) {
            double diff = qAbs(data[medoidIdx][d] - data[idx][d]);
            sum += diff;
        }
        // Lower dispersion = more relevant dimension for this cluster
        dimScores[d] = {sum / k, d};
    }

    // Sort by ascending dispersion (most relevant first)
    std::sort(dimScores.begin(), dimScores.end());

    // Select top l dimensions with lowest dispersion
    int l = qMin(m_avgDimensions, D);
    QVector<int> selected;
    for (int i = 0; i < l; ++i)
        selected.append(dimScores[i].second);

    return selected;
}

/* ---- Greedy medoid initialization ---- */

QVector<int> SubspaceCluster5::greedyInit(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = qMin(m_numClusters, n);
    QVector<int> medoids;
    QVector<bool> used(n, false);

    // Pick first medoid randomly (use index 0 as deterministic seed)
    medoids.append(0);
    used[0] = true;

    // Greedy farthest-first
    for (int m = 1; m < k; ++m) {
        double maxDist = -1.0;
        int bestIdx = -1;
        for (int i = 0; i < n; ++i) {
            if (used[i]) continue;
            double minDist = 1e18;
            for (int med : medoids) {
                double d = 0.0;
                int D = data[0].size();
                for (int dim = 0; dim < D; ++dim)
                    d += qAbs(data[i][dim] - data[med][dim]);
                minDist = qMin(minDist, d / D);
            }
            if (minDist > maxDist) { maxDist = minDist; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            medoids.append(bestIdx);
            used[bestIdx] = true;
        }
    }
    return medoids;
}

/* ---- Point assignment ---- */

QVector<int> SubspaceCluster5::assignPoints(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids,
    const QVector<QVector<int>>& subspaces) const
{
    int n = data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = 1e18;
        for (int c = 0; c < medoids.size(); ++c) {
            double d = manhattanSegmental(data[i], data[medoids[c]], subspaces[c]);
            if (d < bestDist) { bestDist = d; labels[i] = c; }
        }
    }
    return labels;
}

/* ---- Quality computation ---- */

double SubspaceCluster5::computeQuality(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids,
    const QVector<QVector<int>>& subspaces) const
{
    auto labels = assignPoints(data, medoids, subspaces);
    double totalCost = 0.0;
    int n = data.size();
    for (int i = 0; i < n; ++i)
        totalCost += manhattanSegmental(data[i], data[medoids[labels[i]]],
                                         subspaces[labels[i]]);
    return -totalCost / n;
}

/* ---- Main fit ---- */

SubspaceCluster5::ClusterResult SubspaceCluster5::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;
    int D = data[0].size();
    int k = qMin(m_numClusters, n);

    // Initialize medoids
    auto medoidIdxs = greedyInit(data);
    int m = medoidIdxs.size();
    if (m == 0) return result;

    // Iterative refinement
    QVector<QVector<int>> subspaces(m);
    double bestQuality = -1e18;
    QVector<int> bestMedoids = medoidIdxs;
    QVector<QVector<int>> bestSubspaces;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Step 1: Assign points
        // Step 2: Select dimensions for each medoid
        for (int c = 0; c < m; ++c) {
            QVector<int> neighbors;
            // Collect points likely in this cluster
            for (int i = 0; i < n; ++i) {
                if (i == medoidIdxs[c]) continue;
                double d = 0.0;
                for (int dim = 0; dim < D; ++dim)
                    d += qAbs(data[i][dim] - data[medoidIdxs[c]][dim]);
                if (d / D < 2.0) // Threshold heuristic
                    neighbors.append(i);
            }
            if (neighbors.isEmpty()) {
                // Fallback: use nearest neighbors
                for (int i = 0; i < qMin(n - 1, qMax(5, n / k)); ++i) {
                    if (i != medoidIdxs[c]) neighbors.append(i);
                }
            }
            subspaces[c] = selectDimensions(data, medoidIdxs[c], neighbors);
        }

        // Step 3: Assign and update medoids
        auto labels = assignPoints(data, medoidIdxs, subspaces);

        // Update medoid per cluster
        for (int c = 0; c < m; ++c) {
            double bestCost = 1e18;
            int bestMed = medoidIdxs[c];
            for (int i = 0; i < n; ++i) {
                if (labels[i] != c) continue;
                double cost = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (labels[j] != c) continue;
                    cost += manhattanSegmental(data[i], data[j], subspaces[c]);
                }
                if (cost < bestCost) { bestCost = cost; bestMed = i; }
            }
            medoidIdxs[c] = bestMed;
        }

        double q = computeQuality(data, medoidIdxs, subspaces);
        if (q > bestQuality) {
            bestQuality = q;
            bestMedoids = medoidIdxs;
            bestSubspaces = subspaces;
        }
    }

    // Build final result
    result.subspaceDims = bestSubspaces;
    result.medoids.resize(bestMedoids.size());
    for (int i = 0; i < bestMedoids.size(); ++i)
        result.medoids[i] = data[bestMedoids[i]];
    result.labels = assignPoints(data, bestMedoids, bestSubspaces);

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numClusters = bestMedoids.size();
    m_stats.totalDimensions = D;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(bestMedoids.size(), bestQuality);
    return result;
}

/* ---- Reset ---- */

void SubspaceCluster5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
