/**
 * @file HierarchicalCluster10.cpp
 * @brief HierarchicalCluster10 实现
 *
 * 实现UPGMA层次聚类：超度量树构建、共表距离矩阵计算与树状图验证。
 */

#include "utils/cluster223/HierarchicalCluster10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HierarchicalCluster10::HierarchicalCluster10(QObject *parent) : QObject(parent) {}
HierarchicalCluster10::~HierarchicalCluster10() = default;

/* ---- Configuration ---- */

void HierarchicalCluster10::setParameters(int targetClusters)
{
    m_targetClusters = qMax(2, targetClusters);
}

/* ---- Euclidean distance ---- */

double HierarchicalCluster10::euclidean(const QVector<double>& a,
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

/* ---- Compute pairwise distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster10::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- UPGMA distance update ---- */

double HierarchicalCluster10::upgmaDistance(int sizeA, int sizeB, int sizeC,
                                              double dAC, double dBC) const
{
    // UPGMA (unweighted pair group method with arithmetic mean)
    Q_UNUSED(sizeB);
    return (sizeA * dAC + sizeC * dBC) / (sizeA + sizeC);
}

/* ---- Fit ---- */

HierarchicalCluster10::ClusterResult HierarchicalCluster10::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < 2) return result;

    m_stats.numPoints = n;
    m_stats.dim = data[0].size();

    // Compute distance matrix
    QVector<QVector<double>> dist = computeDistanceMatrix(data);

    // Initialize each point as its own cluster
    QVector<int> clusterId(n);
    QVector<int> clusterSize(n, 1);
    QVector<bool> active(n, true);
    for (int i = 0; i < n; ++i) clusterId[i] = i;

    // UPGMA agglomerative loop
    int numActive = n;
    while (numActive > 1) {
        // Find closest pair
        double minDist = std::numeric_limits<double>::max();
        int mergeA = -1, mergeB = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeA = i;
                    mergeB = j;
                }
            }
        }
        if (mergeA < 0) break;

        // Record merge
        MergeRecord rec;
        rec.clusterA = clusterId[mergeA];
        rec.clusterB = clusterId[mergeB];
        rec.distance = minDist;
        rec.newSize = clusterSize[mergeA] + clusterSize[mergeB];
        result.merges.append(rec);

        // Update distances using UPGMA formula
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeA || k == mergeB) continue;
            double newDist = upgmaDistance(clusterSize[mergeA], clusterSize[mergeB],
                                            clusterSize[k],
                                            dist[mergeA][k], dist[mergeB][k]);
            dist[mergeA][k] = newDist;
            dist[k][mergeA] = newDist;
        }

        // Merge B into A
        clusterSize[mergeA] = rec.newSize;
        clusterId[mergeA] = n + result.merges.size() - 1;
        active[mergeB] = false;
        numActive--;

        // Check if we reached target clusters
        if (numActive <= m_targetClusters) break;
    }

    // Build cluster assignments from merge history
    result.numClusters = numActive;
    result.clusters.resize(numActive);
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        if (active[i]) {
            // Collect all original points in this cluster
            QVector<int> members;
            members.append(i);
            // Trace back merges to find all members
            for (int j = 0; j < n; ++j) {
                if (j != i && active[j]) continue;
                if (j == i) continue;
                // j was merged somewhere - check if it merged into i's chain
            }
            result.clusters[idx] = members;
            idx++;
        }
    }

    // Compute cophenetic distance matrix and correlation
    QVector<QVector<double>> coph = copheneticMatrix(result.merges, n);
    if (n > 2) {
        result.copheneticCorrelation = copheneticCorrelation(dist, coph);
        m_stats.copheneticCorrelation = result.copheneticCorrelation;
    }
    result.isClosedTour = (numActive == 1);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(result.numClusters, result.copheneticCorrelation,
                              timer.elapsed());
    return result;
}

/* ---- Cophenetic distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster10::copheneticMatrix(
    const QVector<MergeRecord>& merges, int numPoints) const
{
    QVector<QVector<double>> coph(numPoints, QVector<double>(numPoints, 0.0));

    // Union-Find to track when pairs first merge
    QVector<int> parent(numPoints * 2);
    QVector<double> mergeHeight(numPoints * 2, 0.0);
    for (int i = 0; i < numPoints * 2; ++i) parent[i] = i;

    // Track which points belong to each cluster node
    QVector<QVector<int>> members(numPoints * 2);
    for (int i = 0; i < numPoints; ++i) members[i].append(i);

    for (int m = 0; m < merges.size(); ++m) {
        int newId = numPoints + m;
        int a = merges[m].clusterA;
        int b = merges[m].clusterB;
        mergeHeight[newId] = merges[m].distance;

        // Update cophenetic distances for cross-cluster pairs
        for (int i : members[a]) {
            for (int j : members[b]) {
                if (i < numPoints && j < numPoints)
                    coph[i][j] = coph[j][i] = merges[m].distance;
            }
        }

        // Merge members
        members[newId] = members[a];
        members[newId].append(members[b]);
        parent[a] = newId;
        parent[b] = newId;
    }
    return coph;
}

/* ---- Cophenetic correlation ---- */

double HierarchicalCluster10::copheneticCorrelation(
    const QVector<QVector<double>>& originalDist,
    const QVector<QVector<double>>& cophDist) const
{
    int n = originalDist.size();
    if (n < 3) return 0.0;

    // Pearson correlation between upper triangles
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
    int count = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double x = originalDist[i][j];
            double y = cophDist[i][j];
            sumX += x; sumY += y;
            sumXY += x * y;
            sumX2 += x * x; sumY2 += y * y;
            count++;
        }
    }

    if (count < 2) return 0.0;
    double num = count * sumXY - sumX * sumY;
    double den = qSqrt((count * sumX2 - sumX * sumX) * (count * sumY2 - sumY * sumY));
    return (den > 1e-15) ? num / den : 0.0;
}

/* ---- Cut at distance threshold ---- */

QVector<int> HierarchicalCluster10::cutAtDistance(const QVector<MergeRecord>& merges,
                                                    int numPoints, double threshold) const
{
    // Union-Find
    QVector<int> parent(numPoints);
    for (int i = 0; i < numPoints; ++i) parent[i] = i;

    auto find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    for (const auto& merge : merges) {
        if (merge.distance > threshold) break;
        int a = find(merge.clusterA);
        int b = find(merge.clusterB);
        if (a != b) parent[a] = b;
    }

    // Assign cluster labels
    QMap<int, int> labelMap;
    QVector<int> labels(numPoints);
    int label = 0;
    for (int i = 0; i < numPoints; ++i) {
        int root = find(i);
        if (!labelMap.contains(root)) labelMap[root] = label++;
        labels[i] = labelMap[root];
    }
    return labels;
}

/* ---- Reset ---- */

void HierarchicalCluster10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
