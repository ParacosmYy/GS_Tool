/**
 * @file SubspaceCluster9.cpp
 * @brief SubspaceCluster9 实现
 *
 * 实现子空间聚类：PROCLUS轴平行投影与质量引导中心点精炼。
 */

#include "utils/cluster238/SubspaceCluster9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

SubspaceCluster9::SubspaceCluster9(QObject *parent) : QObject(parent) {}
SubspaceCluster9::~SubspaceCluster9() = default;

/* ---- Configuration ---- */

void SubspaceCluster9::setNumClusters(int k) { m_k = qMax(2, k); }
void SubspaceCluster9::setSubspaceSize(int l) { m_l = qMax(1, l); }

/* ---- Manhattan segmental distance ---- */

double SubspaceCluster9::manhattanDist(const QVector<double>& a, const QVector<double>& b,
                                        const QVector<int>& dims) const
{
    double sum = 0.0;
    for (int d : dims) {
        if (d >= 0 && d < a.size() && d < b.size())
            sum += qAbs(a[d] - b[d]);
    }
    return sum / dims.size();
}

/* ---- Quality metric ---- */

double SubspaceCluster9::computeQuality(const QVector<int>& medoids,
                                         const QVector<QVector<int>>& subspaces) const
{
    int n = m_data.size();
    double totalDist = 0.0;
    int count = 0;

    for (int i = 0; i < n; ++i) {
        double bestDist = 1e18;
        for (int m = 0; m < medoids.size(); ++m) {
            double d = manhattanDist(m_data[i], m_data[medoids[m]], subspaces[m]);
            bestDist = qMin(bestDist, d);
        }
        totalDist += bestDist;
        count++;
    }
    return (count > 0) ? -totalDist / count : 0.0;
}

/* ---- Initial medoid selection (farthest-first) ---- */

QVector<int> SubspaceCluster9::selectInitialMedoids(int n) const
{
    QVector<int> medoids;
    if (n <= m_k) {
        for (int i = 0; i < n; ++i) medoids.append(i);
        return medoids;
    }

    std::mt19937 rng(42);
    int first = rng() % n;
    medoids.append(first);

    QVector<double> minDist(n, 1e18);
    for (int k = 1; k < m_k; ++k) {
        // Update distances to nearest selected medoid
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            int dim = m_data[0].size();
            for (int j = 0; j < dim; ++j)
                d += qAbs(m_data[i][j] - m_data[medoids.last()][j]);
            minDist[i] = qMin(minDist[i], d);
        }
        // Select farthest point
        int best = -1;
        double bestDist = -1;
        for (int i = 0; i < n; ++i) {
            if (!medoids.contains(i) && minDist[i] > bestDist) {
                bestDist = minDist[i];
                best = i;
            }
        }
        if (best >= 0) medoids.append(best);
    }
    return medoids;
}

/* ---- Find subspace dimensions for a medoid ---- */

QVector<int> SubspaceCluster9::findSubspace(int medoid, const QVector<int>& neighbors) const
{
    int d = m_data[0].size();
    if (neighbors.isEmpty()) {
        QVector<int> all(d);
        for (int i = 0; i < d; ++i) all[i] = i;
        return all.mid(0, qMin(m_l, d));
    }

    // Compute variance per dimension among neighbors
    QVector<double> variance(d, 0.0);
    for (int dim = 0; dim < d; ++dim) {
        double mean = 0.0;
        for (int ni : neighbors)
            mean += m_data[ni][dim];
        mean /= neighbors.size();
        for (int ni : neighbors) {
            double diff = m_data[ni][dim] - mean;
            variance[dim] += diff * diff;
        }
        variance[dim] /= neighbors.size();
    }

    // Sort dimensions by ascending variance (low variance = good subspace dimension)
    QVector<int> indices(d);
    for (int i = 0; i < d; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return variance[a] < variance[b];
    });

    int l = qMin(m_l * 2, d);
    return indices.mid(0, l);
}

/* ---- Fit ---- */

QVector<SubspaceCluster9::ClusterResult> SubspaceCluster9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n < m_k) return {};

    int d = data[0].size();
    m_stats.numPoints = n;
    m_stats.numDimensions = d;

    // Initialize medoids
    QVector<int> medoids = selectInitialMedoids(n);
    QVector<QVector<int>> subspaces(m_k);

    // Assign points to nearest medoid, find subspaces
    m_labels.resize(n);
    double bestQuality = -1e18;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Assign each point to nearest medoid
        QVector<QVector<int>> clusters(m_k);
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestM = 0;
            for (int m = 0; m < medoids.size(); ++m) {
                QVector<int> dims = subspaces[m].isEmpty()
                    ? [&]() { QVector<int> all(d); for (int j = 0; j < d; ++j) all[j] = j; return all; }()
                    : subspaces[m];
                double dist = manhattanDist(data[i], data[medoids[m]], dims);
                if (dist < bestDist) { bestDist = dist; bestM = m; }
            }
            clusters[bestM].append(i);
            m_labels[i] = bestM;
        }

        // Find subspace for each medoid
        for (int m = 0; m < medoids.size(); ++m)
            subspaces[m] = findSubspace(medoids[m], clusters[m]);

        // Quality-guided medoid refinement: swap worst medoid
        double q = computeQuality(medoids, subspaces);
        if (q > bestQuality) {
            bestQuality = q;
        } else {
            // Try swapping a random medoid
            int swapM = iter % medoids.size();
            if (!clusters[swapM].isEmpty()) {
                int newMedoid = clusters[swapM][clusters[swapM].size() / 2];
                medoids[swapM] = newMedoid;
            }
        }

        m_stats.numIterations = iter + 1;
        emit iterationCompleted(iter + 1, bestQuality);

        if (timer.elapsed() > 5000) break;
    }

    // Build results
    m_clusters.clear();
    QVector<QVector<int>> finalClusters(m_k);
    for (int i = 0; i < n; ++i)
        if (m_labels[i] >= 0 && m_labels[i] < m_k)
            finalClusters[m_labels[i]].append(i);

    for (int m = 0; m < m_k; ++m) {
        ClusterResult cr;
        cr.pointIndices = finalClusters[m];
        cr.subspaceDims = subspaces[m];
        cr.quality = bestQuality;
        m_clusters.append(cr);
    }

    m_stats.numClusters = m_k;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_k, m_stats.numIterations, timer.elapsed());
    return m_clusters;
}

/* ---- Accessors ---- */

QVector<int> SubspaceCluster9::labels() const { return m_labels; }

/* ---- Reset ---- */

void SubspaceCluster9::resetStatistics()
{
    m_data.clear(); m_labels.clear(); m_clusters.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
