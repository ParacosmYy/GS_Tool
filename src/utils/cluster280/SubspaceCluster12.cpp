/**
 * @file SubspaceCluster12.cpp
 * @brief SubspaceCluster12 实现
 *
 * 实现子空间聚类：PROCLUS中心点投影与k-medoid精炼的轴平行子空间发现。
 */

#include "utils/cluster280/SubspaceCluster12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SubspaceCluster12::SubspaceCluster12(QObject *parent)
    : QObject(parent) {}

SubspaceCluster12::~SubspaceCluster12() = default;

/* ---- Configuration ---- */

void SubspaceCluster12::setNumClusters(int k) { m_k = qBound(2, k, 100); }
void SubspaceCluster12::setAvgSubspaceDim(int l) { m_l = qBound(1, l, 50); }

/* ---- Subspace distance ---- */

double SubspaceCluster12::subspaceDistance(const QVector<double>& a,
                                            const QVector<double>& b,
                                            const QVector<int>& dims) const
{
    double sum = 0.0;
    for (int d : dims) {
        if (d < a.size() && d < b.size()) {
            double diff = a[d] - b[d];
            sum += diff * diff;
        }
    }
    return qSqrt(sum);
}

/* ---- Greedy farthest-first medoid selection ---- */

QVector<int> SubspaceCluster12::selectMedoids(const QVector<QVector<double>>& data,
                                                int k) const
{
    int n = data.size();
    if (n == 0) return {};

    QVector<int> medoids;
    QVector<double> minDist(n, 1e30);

    // Start with a random-ish first medoid (use first point)
    medoids.append(0);
    for (int i = 0; i < n; ++i)
        minDist[i] = subspaceDistance(data[0], data[i], {});

    // All-dimension distance for initialization
    QVector<int> allDims(data.isEmpty() ? 0 : data[0].size());
    for (int i = 0; i < allDims.size(); ++i) allDims[i] = i;

    for (int m = 1; m < k && m < n; ++m) {
        // Find farthest point from current medoids
        double maxDist = -1.0;
        int bestIdx = -1;
        for (int i = 0; i < n; ++i) {
            double d = subspaceDistance(data[medoids.last()], data[i], allDims);
            minDist[i] = qMin(minDist[i], d);
            if (minDist[i] > maxDist) {
                maxDist = minDist[i];
                bestIdx = i;
            }
        }
        if (bestIdx >= 0) medoids.append(bestIdx);
    }
    return medoids;
}

/* ---- Find relevant dimensions for each medoid ---- */

QVector<QVector<int>> SubspaceCluster12::findRelevantDimensions(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids) const
{
    int n = data.size();
    int d = data.isEmpty() ? 0 : data[0].size();
    int k = medoids.size();
    int l = qMin(m_l, d);

    // Compute per-dimension average distance to nearest medoid
    QVector<double> dimAvgDist(d, 0.0);
    for (int dim = 0; dim < d; ++dim) {
        QVector<int> singleDim = {dim};
        for (int i = 0; i < n; ++i) {
            double minD = 1e30;
            for (int m = 0; m < k; ++m)
                minD = qMin(minD, subspaceDistance(data[i], data[medoids[m]], singleDim));
            dimAvgDist[dim] += minD;
        }
        dimAvgDist[dim] /= n;
    }

    // For each medoid, select l dimensions with smallest local variance
    QVector<QVector<int>> subspaces(k);
    for (int m = 0; m < k; ++m) {
        // Compute variance of each dimension around this medoid
        QVector<QPair<double, int>> dimScore(d);
        for (int dim = 0; dim < d; ++dim) {
            double variance = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i][dim] - data[medoids[m]][dim];
                variance += diff * diff;
            }
            // Score: low variance + low avg distance = relevant
            dimScore[dim] = {variance / n + dimAvgDist[dim] * 0.1, dim};
        }
        std::sort(dimScore.begin(), dimScore.end());
        for (int j = 0; j < l && j < d; ++j)
            subspaces[m].append(dimScore[j].second);
    }
    return subspaces;
}

/* ---- Assign points to nearest medoid in subspace ---- */

QVector<int> SubspaceCluster12::assignPoints(const QVector<QVector<double>>& data,
                                               const QVector<int>& medoids,
                                               const QVector<QVector<int>>& subspaces) const
{
    int n = data.size();
    int k = medoids.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double minDist = 1e30;
        for (int m = 0; m < k; ++m) {
            double d = subspaceDistance(data[i], data[medoids[m]], subspaces[m]);
            if (d < minDist) {
                minDist = d;
                labels[i] = m;
            }
        }
    }
    return labels;
}

/* ---- Refine medoids using k-medoid swaps ---- */

QVector<int> SubspaceCluster12::refineMedoids(const QVector<QVector<double>>& data,
                                                const QVector<int>& medoids,
                                                const QVector<int>& labels,
                                                const QVector<QVector<int>>& subspaces) const
{
    int k = medoids.size();
    QVector<int> newMedoids = medoids;

    for (int m = 0; m < k; ++m) {
        // Collect cluster members
        QVector<int> members;
        for (int i = 0; i < labels.size(); ++i)
            if (labels[i] == m) members.append(i);

        if (members.isEmpty()) continue;

        // Find best medoid: minimize sum of distances to all members
        double bestCost = 1e30;
        int bestIdx = newMedoids[m];
        for (int candidate : members) {
            double cost = 0.0;
            for (int member : members)
                cost += subspaceDistance(data[candidate], data[member], subspaces[m]);
            if (cost < bestCost) {
                bestCost = cost;
                bestIdx = candidate;
            }
        }
        newMedoids[m] = bestIdx;
    }
    return newMedoids;
}

/* ---- Compute cluster quality ---- */

double SubspaceCluster12::computeQuality(const QVector<QVector<double>>& data,
                                           const QVector<int>& medoids,
                                           const QVector<QVector<int>>& subspaces) const
{
    int k = medoids.size();
    int n = data.size();
    if (n == 0 || k == 0) return 0.0;

    // Quality = average intra-cluster compactness (lower is better, return negative)
    double totalCompact = 0.0;
    QVector<int> labels = assignPoints(data, medoids, subspaces);

    for (int m = 0; m < k; ++m) {
        int count = 0;
        double sumDist = 0.0;
        for (int i = 0; i < n; ++i) {
            if (labels[i] == m) {
                sumDist += subspaceDistance(data[i], data[medoids[m]], subspaces[m]);
                count++;
            }
        }
        if (count > 0) totalCompact += sumDist / count;
    }
    return -totalCompact / k;  // Negative: higher quality = less compactness cost
}

/* ---- Main PROCLUS fitting ---- */

SubspaceCluster12::ClusterResult SubspaceCluster12::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < m_k) return result;

    // Phase 1: Initial medoid selection
    QVector<int> medoids = selectMedoids(data, m_k);

    // Phase 2: Iterative refinement
    const int maxIter = 20;
    for (int iter = 0; iter < maxIter; ++iter) {
        // Find relevant subspaces for current medoids
        auto subspaces = findRelevantDimensions(data, medoids);

        // Assign points
        auto labels = assignPoints(data, medoids, subspaces);

        // Refine medoids
        QVector<int> newMedoids = refineMedoids(data, medoids, labels, subspaces);

        // Check convergence
        bool converged = (newMedoids == medoids);
        medoids = newMedoids;
        result.numIterations = iter + 1;

        double quality = computeQuality(data, medoids, subspaces);
        double elapsed = timer.elapsed();
        emit iterationDone(iter, m_k, quality, elapsed);

        if (converged) break;
    }

    // Final assignment and result construction
    auto finalSubspaces = findRelevantDimensions(data, medoids);
    auto finalLabels = assignPoints(data, medoids, finalSubspaces);
    result.totalQuality = computeQuality(data, medoids, finalSubspaces);

    for (int m = 0; m < m_k; ++m) {
        SubspaceCluster sc;
        sc.relevantDimensions = finalSubspaces[m];
        for (int i = 0; i < n; ++i)
            if (finalLabels[i] == m) sc.pointIndices.append(i);
        sc.quality = 0.0;
        for (int idx : sc.pointIndices)
            sc.quality += subspaceDistance(data[idx], data[medoids[m]], sc.relevantDimensions);
        if (!sc.pointIndices.isEmpty()) sc.quality /= sc.pointIndices.size();
        result.clusters.append(sc);
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numDimensions = data.isEmpty() ? 0 : data[0].size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(n, m_k, result.totalQuality, elapsed);

    return result;
}

/* ---- Reset ---- */

void SubspaceCluster12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
