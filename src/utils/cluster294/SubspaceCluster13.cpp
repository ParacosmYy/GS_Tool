/**
 * @file SubspaceCluster13.cpp
 * @brief SubspaceCluster13 实现
 *
 * 实现子空间聚类：PROCLUS维度选择与K-中心点精炼实现高维投影聚类发现。
 */

#include "utils/cluster294/SubspaceCluster13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SubspaceCluster13::SubspaceCluster13(QObject *parent)
    : QObject(parent) {}

SubspaceCluster13::~SubspaceCluster13() = default;

/* ---- Configuration ---- */

void SubspaceCluster13::setNumClusters(int k) { m_k = qBound(2, k, 200); }
void SubspaceCluster13::setAvgDimSelection(int l) { m_l = qBound(1, l, 1000); }

/* ---- Manhattan segmental distance ---- */

double SubspaceCluster13::manhattanSegmental(const QVector<double>& a,
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

/* ---- Find initial medoids (farthest-first) ---- */

QVector<int> SubspaceCluster13::findInitialMedoids(
    const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    if (n == 0) return {};
    QVector<int> medoids;
    medoids.append(0);

    // Track min distance to any chosen medoid
    QVector<double> minDist(n, 1e300);
    for (int i = 0; i < n; ++i) {
        double d = 0.0;
        int dim = qMin(data[0].size(), data[i].size());
        for (int j = 0; j < dim; ++j)
            d += qAbs(data[0][j] - data[i][j]);
        minDist[i] = d;
    }

    // Greedy farthest-first selection
    for (int iter = 1; iter < k && iter < n; ++iter) {
        int bestIdx = -1;
        double bestDist = -1.0;
        for (int i = 0; i < n; ++i) {
            if (minDist[i] > bestDist) {
                bestDist = minDist[i];
                bestIdx = i;
            }
        }
        if (bestIdx < 0) break;
        medoids.append(bestIdx);

        // Update min distances
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            int dim = qMin(data[bestIdx].size(), data[i].size());
            for (int j = 0; j < dim; ++j)
                d += qAbs(data[bestIdx][j] - data[i][j]);
            minDist[i] = qMin(minDist[i], d);
        }
    }
    return medoids;
}

/* ---- PROCLUS dimension selection ---- */

QVector<QVector<int>> SubspaceCluster13::selectDimensions(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids) const
{
    int n = data.size();
    int dim = (n > 0) ? data[0].size() : 0;
    int k = medoids.size();
    int l = qMin(m_l, dim);
    if (l <= 0) l = qMax(1, dim / 2);

    QVector<QVector<int>> dimSets(k);

    // For each medoid, find its neighborhood and select dimensions with low variance
    for (int mi = 0; mi < k; ++mi) {
        int med = medoids[mi];
        // Find nearest neighbors to this medoid
        QVector<QPair<double, int>> distIdx;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < dim; ++j)
                d += qAbs(data[med][j] - data[i][j]);
            distIdx.append({d, i});
        }
        std::sort(distIdx.begin(), distIdx.end());

        // Compute per-dimension average distance in neighborhood
        int neighborhoodSize = qMax(2, n / (k * 2));
        QVector<double> dimAvg(dim, 0.0);
        for (int d = 0; d < dim; ++d) {
            double sum = 0.0;
            for (int ni = 0; ni < neighborhoodSize && ni < distIdx.size(); ++ni)
                sum += qAbs(data[med][d] - data[distIdx[ni].second][d]);
            dimAvg[d] = sum / neighborhoodSize;
        }

        // Select dimensions with smallest average deviation (most tightly clustered)
        QVector<QPair<double, int>> dimScores;
        for (int d = 0; d < dim; ++d)
            dimScores.append({dimAvg[d], d});
        std::sort(dimScores.begin(), dimScores.end());

        int selCount = qMin(l, dimScores.size());
        for (int i = 0; i < selCount; ++i)
            dimSets[mi].append(dimScores[i].second);
    }
    return dimSets;
}

/* ---- Assign points to medoids in projected subspaces ---- */

QVector<int> SubspaceCluster13::assignPoints(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids,
    const QVector<QVector<int>>& dimSets) const
{
    int n = data.size();
    int k = medoids.size();
    QVector<int> assignments(n, 0);

    for (int i = 0; i < n; ++i) {
        double bestDist = 1e300;
        int bestCluster = 0;
        for (int mi = 0; mi < k; ++mi) {
            double d = manhattanSegmental(data[i], data[medoids[mi]], dimSets[mi]);
            if (d < bestDist) {
                bestDist = d;
                bestCluster = mi;
            }
        }
        assignments[i] = bestCluster;
    }
    return assignments;
}

/* ---- Refine medoids via k-medoid swap ---- */

QVector<int> SubspaceCluster13::refineMedoids(
    const QVector<QVector<double>>& data,
    const QVector<int>& medoids,
    const QVector<int>& assignments) const
{
    int k = medoids.size();
    int dim = (data.size() > 0) ? data[0].size() : 0;
    QVector<int> refined = medoids;

    for (int mi = 0; mi < k; ++mi) {
        // Collect points in this cluster
        QVector<int> members;
        for (int i = 0; i < assignments.size(); ++i)
            if (assignments[i] == mi) members.append(i);
        if (members.isEmpty()) continue;

        // Find best medoid: point minimizing total Manhattan distance to all members
        double bestCost = 1e300;
        int bestIdx = refined[mi];
        for (int candidate : members) {
            double cost = 0.0;
            for (int member : members) {
                for (int d = 0; d < dim; ++d)
                    cost += qAbs(data[candidate][d] - data[member][d]);
            }
            if (cost < bestCost) {
                bestCost = cost;
                bestIdx = candidate;
            }
        }
        refined[mi] = bestIdx;
    }
    return refined;
}

/* ---- Compute cluster quality score ---- */

double SubspaceCluster13::computeQuality(
    const QVector<QVector<double>>& data,
    const QVector<int>& assignments,
    const QVector<QVector<int>>& dimSets) const
{
    int n = data.size();
    int k = dimSets.size();
    if (n == 0) return 0.0;

    double totalSilhouette = 0.0;
    for (int i = 0; i < n; ++i) {
        int ci = assignments[i];
        // Intra-cluster distance
        double intraSum = 0.0;
        int intraCount = 0;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            if (assignments[j] == ci) {
                intraSum += manhattanSegmental(data[i], data[j], dimSets[ci]);
                intraCount++;
            }
        }
        double a = (intraCount > 0) ? intraSum / intraCount : 0.0;

        // Nearest-cluster distance
        double minInter = 1e300;
        for (int cj = 0; cj < k; ++cj) {
            if (cj == ci) continue;
            double interSum = 0.0;
            int interCount = 0;
            for (int j = 0; j < n; ++j) {
                if (assignments[j] == cj) {
                    interSum += manhattanSegmental(data[i], data[j], dimSets[cj]);
                    interCount++;
                }
            }
            double b = (interCount > 0) ? interSum / interCount : 0.0;
            minInter = qMin(minInter, b);
        }
        double s = (minInter < 1e300 && (a + minInter) > 0)
                       ? (minInter - a) / qMax(a, minInter)
                       : 0.0;
        totalSilhouette += s;
    }
    return totalSilhouette / n;
}

/* ---- Fit ---- */

SubspaceCluster13::ClusterResult SubspaceCluster13::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    ClusterResult result;

    if (n == 0) {
        result.numClusters = 0;
        return result;
    }

    int dim = data[0].size();
    int k = qMin(m_k, n);

    // Phase 1: Initialize medoids
    auto medoids = findInitialMedoids(data, k);

    // Phase 2: Iterative PROCLUS refinement (3 iterations)
    for (int iter = 0; iter < 3; ++iter) {
        // Select dimensions for each medoid
        auto dimSets = selectDimensions(data, medoids);

        // Assign points to clusters
        auto assignments = assignPoints(data, medoids, dimSets);

        // Refine medoids
        medoids = refineMedoids(data, medoids, assignments);
    }

    // Final pass: compute dimSets and assignments with refined medoids
    auto finalDimSets = selectDimensions(data, medoids);
    auto finalAssignments = assignPoints(data, medoids, finalDimSets);

    // Build result clusters
    result.numClusters = medoids.size();
    result.assignments = finalAssignments;

    for (int mi = 0; mi < medoids.size(); ++mi) {
        ProjectedCluster pc;
        pc.medoid = data[medoids[mi]];
        pc.selectedDims = finalDimSets[mi];
        for (int i = 0; i < n; ++i) {
            if (finalAssignments[i] == mi) {
                pc.pointIndices.append(i);
                pc.avgDistance += manhattanSegmental(
                    data[i], data[medoids[mi]], finalDimSets[mi]);
            }
        }
        pc.avgDistance = pc.pointIndices.isEmpty()
                             ? 0.0
                             : pc.avgDistance / pc.pointIndices.size();
        result.clusters.append(pc);
    }

    result.qualityScore = computeQuality(data, finalAssignments, finalDimSets);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numDims = dim;
    m_stats.totalFits++;
    m_qualitySum += result.qualityScore;
    m_stats.avgQuality = m_qualitySum / m_stats.totalFits;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(n, k, result.qualityScore, elapsed);
    return result;
}

/* ---- Reset ---- */

void SubspaceCluster13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_qualitySum = 0.0;
}
