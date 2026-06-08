/**
 * @file SubspaceCluster8.cpp
 * @brief SubspaceCluster8 实现
 *
 * 实现INSCY密度子空间聚类与冗余感知子空间选择。
 */

#include "utils/cluster224/SubspaceCluster8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SubspaceCluster8::SubspaceCluster8(QObject *parent) : QObject(parent) {}
SubspaceCluster8::~SubspaceCluster8() = default;

/* ---- Configuration ---- */

void SubspaceCluster8::setParameters(double minDensity, int minPoints,
                                      double redundancyThreshold,
                                      int maxSubspaceDim)
{
    m_minDensity = qBound(0.01, minDensity, 1.0);
    m_minPoints = qMax(2, minPoints);
    m_redundancyThreshold = qBound(0.0, redundancyThreshold, 1.0);
    m_maxSubspaceDim = qMax(0, maxSubspaceDim);
}

/* ---- Euclidean distance ---- */

double SubspaceCluster8::euclidean(const QVector<double>& a,
                                    const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Project data onto dimensions ---- */

QVector<QVector<double>> SubspaceCluster8::project(
    const QVector<QVector<double>>& data,
    const QVector<int>& dims) const
{
    QVector<QVector<double>> projected(data.size());
    for (int i = 0; i < data.size(); ++i) {
        projected[i].resize(dims.size());
        for (int d = 0; d < dims.size(); ++d)
            projected[i][d] = data[i][dims[d]];
    }
    return projected;
}

/* ---- INSCY density clustering ---- */

QVector<QVector<int>> SubspaceCluster8::inscyCluster(
    const QVector<QVector<double>>& projected, double epsilon) const
{
    int n = projected.size();
    if (n == 0) return {};

    // Compute pairwise distances and find density-connected components
    QVector<int> labels(n, -1);
    QVector<bool> core(n, false);
    int clusterId = 0;

    // Find core points (density >= minDensity threshold)
    for (int i = 0; i < n; ++i) {
        int neighbors = 0;
        for (int j = 0; j < n; ++j) {
            if (i != j && euclidean(projected[i], projected[j]) <= epsilon)
                neighbors++;
        }
        core[i] = (neighbors >= m_minPoints);
    }

    // BFS expansion from core points
    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0 || !core[i]) continue;
        labels[i] = clusterId;
        QVector<int> queue;
        queue.append(i);
        int head = 0;
        while (head < queue.size()) {
            int cur = queue[head++];
            for (int j = 0; j < n; ++j) {
                if (labels[j] >= 0) continue;
                if (euclidean(projected[cur], projected[j]) <= epsilon) {
                    labels[j] = clusterId;
                    if (core[j]) queue.append(j);
                }
            }
        }
        clusterId++;
    }

    // Collect clusters
    QVector<QVector<int>> clusters(clusterId);
    for (int i = 0; i < n; ++i)
        if (labels[i] >= 0) clusters[labels[i]].append(i);
    return clusters;
}

/* ---- Generate subspaces ---- */

QVector<QVector<int>> SubspaceCluster8::generateSubspaces(
    int fullDim, int maxDim) const
{
    int limit = (maxDim > 0) ? qMin(maxDim, fullDim) : fullDim;
    QVector<QVector<int>> result;

    // Generate all combinations of 1..limit dimensions
    for (int d = 1; d <= limit; ++d) {
        QVector<int> combo(d);
        for (int i = 0; i < d; ++i) combo[i] = i;
        while (true) {
            result.append(combo);
            int k = d - 1;
            while (k >= 0 && combo[k] == fullDim - d + k) k--;
            if (k < 0) break;
            combo[k]++;
            for (int j = k + 1; j < d; ++j) combo[j] = combo[j - 1] + 1;
        }
    }
    return result;
}

/* ---- Cluster quality ---- */

double SubspaceCluster8::clusterQuality(
    const QVector<QVector<double>>& projected,
    const QVector<QVector<int>>& clusters) const
{
    if (clusters.isEmpty()) return 0.0;
    int n = projected.size();
    if (n < 2) return 0.0;

    // Silhouette-like quality measure
    double totalSil = 0.0;
    int count = 0;
    for (const auto& cl : clusters) {
        if (cl.size() < 2) continue;
        // Intra-cluster average distance
        double intraSum = 0.0;
        for (int i : cl)
            for (int j : cl)
                if (i != j) intraSum += euclidean(projected[i], projected[j]);
        double intra = intraSum / (cl.size() * (cl.size() - 1));

        // Nearest other cluster distance
        double minInter = std::numeric_limits<double>::max();
        for (const auto& other : clusters) {
            if (&other == &cl) continue;
            double interSum = 0.0;
            int interCount = 0;
            for (int i : cl)
                for (int j : other) {
                    interSum += euclidean(projected[i], projected[j]);
                    interCount++;
                }
            if (interCount > 0) minInter = qMin(minInter, interSum / interCount);
        }
        if (minInter < std::numeric_limits<double>::max()) {
            double sil = (minInter - intra) / qMax(minInter, intra);
            totalSil += sil;
            count++;
        }
    }
    return (count > 0) ? totalSil / count : 0.0;
}

/* ---- Redundancy (Jaccard similarity) ---- */

double SubspaceCluster8::redundancy(const SubspaceCluster& a,
                                     const SubspaceCluster& b) const
{
    // Jaccard on dimension overlap
    int dimOverlap = 0;
    for (int d : a.dimensions)
        for (int e : b.dimensions)
            if (d == e) { dimOverlap++; break; }
    int dimUnion = a.dimensions.size() + b.dimensions.size() - dimOverlap;
    double dimJac = (dimUnion > 0) ? static_cast<double>(dimOverlap) / dimUnion : 0.0;

    // Jaccard on point overlap (across all clusters)
    QSet<int> pointsA, pointsB;
    for (const auto& cl : a.clusters)
        for (int p : cl) pointsA.insert(p);
    for (const auto& cl : b.clusters)
        for (int p : cl) pointsB.insert(p);
    int ptOverlap = 0;
    for (int p : pointsA)
        if (pointsB.contains(p)) ptOverlap++;
    int ptUnion = pointsA.size() + pointsB.size() - ptOverlap;
    double ptJac = (ptUnion > 0) ? static_cast<double>(ptOverlap) / ptUnion : 0.0;

    return qMax(dimJac, ptJac);
}

/* ---- Select non-redundant ---- */

QVector<SubspaceCluster8::SubspaceCluster> SubspaceCluster8::selectNonRedundant(
    const QVector<SubspaceCluster>& candidates) const
{
    // Sort by quality descending
    QVector<SubspaceCluster> sorted = candidates;
    std::sort(sorted.begin(), sorted.end(),
              [](const SubspaceCluster& a, const SubspaceCluster& b) {
                  return a.quality > b.quality;
              });

    QVector<SubspaceCluster> selected;
    for (const auto& cand : sorted) {
        bool redundant = false;
        for (const auto& sel : selected) {
            if (redundancy(sel, cand) > m_redundancyThreshold) {
                redundant = true;
                break;
            }
        }
        if (!redundant) selected.append(cand);
    }
    return selected;
}

/* ---- Fit ---- */

QVector<SubspaceCluster8::SubspaceCluster> SubspaceCluster8::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return {};
    int fullDim = data[0].size();
    int maxDim = (m_maxSubspaceDim > 0) ? m_maxSubspaceDim : fullDim;

    m_stats.numPoints = n;
    m_stats.fullDim = fullDim;
    m_stats.maxSubspaceDim = maxDim;

    // Generate subspaces
    QVector<QVector<int>> subspaces = generateSubspaces(fullDim, maxDim);
    m_stats.numSubspacesExplored = subspaces.size();

    // Epsilon estimation from data scale
    double globalRange = 0.0;
    for (int d = 0; d < fullDim; ++d) {
        double minV = std::numeric_limits<double>::max();
        double maxV = std::numeric_limits<double>::lowest();
        for (int i = 0; i < n; ++i) {
            minV = qMin(minV, data[i][d]);
            maxV = qMax(maxV, data[i][d]);
        }
        globalRange += (maxV - minV);
    }
    double epsilon = globalRange / fullDim * m_minDensity * 2.0;

    // Cluster in each subspace
    QVector<SubspaceCluster> candidates;
    for (const auto& dims : subspaces) {
        QVector<QVector<double>> proj = project(data, dims);
        QVector<QVector<int>> clusters = inscyCluster(proj, epsilon);
        // Filter small clusters
        QVector<QVector<int>> filtered;
        for (const auto& cl : clusters)
            if (cl.size() >= m_minPoints) filtered.append(cl);
        if (filtered.isEmpty()) continue;

        SubspaceCluster sc;
        sc.dimensions = dims;
        sc.clusters = filtered;
        sc.quality = clusterQuality(proj, filtered);
        candidates.append(sc);
    }

    // Redundancy-aware selection
    QVector<SubspaceCluster> result = selectNonRedundant(candidates);
    m_stats.numClustersFound = 0;
    for (const auto& sc : result)
        m_stats.numClustersFound += sc.clusters.size();

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(result.size(), m_stats.numClustersFound, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SubspaceCluster8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
