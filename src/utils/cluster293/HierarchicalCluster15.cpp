/**
 * @file HierarchicalCluster15.cpp
 * @brief HierarchicalCluster15 实现
 *
 * 实现层次聚类：质心链接与共表型相关系数验证树状图质量。
 */

#include "utils/cluster293/HierarchicalCluster15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HierarchicalCluster15::HierarchicalCluster15(QObject *parent)
    : QObject(parent) {}

HierarchicalCluster15::~HierarchicalCluster15() = default;

/* ---- Configuration ---- */

void HierarchicalCluster15::setNumClusters(int k) { m_k = qBound(1, k, 500); }
void HierarchicalCluster15::setLinkageThreshold(double t) { m_threshold = qBound(0.0, t, 1e300); }

/* ---- Euclidean distance ---- */

double HierarchicalCluster15::euclideanDistance(const QVector<double>& a,
                                                  const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Compute centroid ---- */

QVector<double> HierarchicalCluster15::computeCentroid(
    const QVector<QVector<double>>& data, const QVector<int>& indices) const
{
    if (indices.isEmpty()) return {};
    int dim = data[0].size();
    QVector<double> centroid(dim, 0.0);
    for (int idx : indices) {
        for (int j = 0; j < dim; ++j)
            centroid[j] += data[idx][j];
    }
    double inv = 1.0 / indices.size();
    for (int j = 0; j < dim; ++j)
        centroid[j] *= inv;
    return centroid;
}

/* ---- Compute pairwise distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster15::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclideanDistance(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- Cophenetic correlation coefficient ---- */

double HierarchicalCluster15::copheneticCorrelation(
    const QVector<QVector<double>>& distMatrix,
    const QVector<MergeStep>& merges) const
{
    int n = distMatrix.size();
    if (n < 3) return 0.0;

    // Build cophenetic distance matrix from merge steps
    // Each original point is initially its own cluster
    QVector<int> clusterId(n);
    for (int i = 0; i < n; ++i) clusterId[i] = i;

    // Track which points belong to each cluster at each merge
    QVector<QVector<int>> members(2 * n);
    for (int i = 0; i < n; ++i) members[i] = {i};

    // Map merge distance to cophenetic distance between point pairs
    QVector<QVector<double>> cophDist(n, QVector<double>(n, 0.0));
    int nextId = n;

    for (const auto& merge : merges) {
        int a = merge.clusterA;
        int b = merge.clusterB;
        // Record cophenetic distance between all pairs across clusters
        if (a < 2 * n && b < 2 * n) {
            for (int pa : members[a]) {
                for (int pb : members[b]) {
                    if (pa < n && pb < n) {
                        cophDist[pa][pb] = merge.distance;
                        cophDist[pb][pa] = merge.distance;
                    }
                }
            }
            // Merge members
            members[nextId] = members[a];
            members[nextId].append(members[b]);
        }
        nextId++;
    }

    // Pearson correlation between original and cophenetic distances
    double sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0, sumX = 0.0, sumY = 0.0;
    int count = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double x = distMatrix[i][j];
            double y = cophDist[i][j];
            sumX += x;  sumY += y;
            sumXY += x * y;
            sumX2 += x * x;
            sumY2 += y * y;
            count++;
        }
    }
    if (count == 0) return 0.0;
    double denom = qSqrt((count * sumX2 - sumX * sumX) * (count * sumY2 - sumY * sumY));
    return denom > 0 ? (count * sumXY - sumX * sumY) / denom : 0.0;
}

/* ---- Cut dendrogram ---- */

QVector<int> HierarchicalCluster15::cutDendrogram(const QVector<MergeStep>& merges,
                                                     int n, int k) const
{
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    // Union-Find helper
    auto find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    // Perform n-k merges (merge the first n-k steps)
    int mergesToPerform = qMax(0, n - k);
    QVector<int> root(n);
    for (int i = 0; i < n; ++i) root[i] = i;

    // Remap to union-find on original points
    QVector<int> clusterRoot(2 * n, -1);
    for (int i = 0; i < n; ++i) clusterRoot[i] = i;

    for (int m = 0; m < mergesToPerform && m < merges.size(); ++m) {
        int a = merges[m].clusterA;
        int b = merges[m].clusterB;
        if (a >= 0 && b >= 0 && a < 2 * n && b < 2 * n) {
            int ra = clusterRoot[a] >= 0 ? find(clusterRoot[a]) : -1;
            int rb = clusterRoot[b] >= 0 ? find(clusterRoot[b]) : -1;
            if (ra >= 0 && rb >= 0 && ra != rb)
                parent[rb] = ra;
        }
    }

    QVector<int> assignments(n);
    for (int i = 0; i < n; ++i)
        assignments[i] = find(i);
    return assignments;
}

/* ---- Fit ---- */

HierarchicalCluster15::ClusterResult HierarchicalCluster15::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    ClusterResult result;

    auto distMatrix = computeDistanceMatrix(data);

    // Active cluster tracking
    QVector<bool> active(n, true);
    QVector<QVector<int>> members(n);
    for (int i = 0; i < n; ++i) members[i] = {i};

    // Current centroids (initially data points)
    QVector<QVector<double>> centroids = data;

    // Cluster distance matrix (centroid linkage)
    QVector<QVector<double>> cDist(n, QVector<double>(n, 1e300));
    for (int i = 0; i < n; ++i) {
        cDist[i][i] = 0.0;
        for (int j = i + 1; j < n; ++j) {
            cDist[i][j] = distMatrix[i][j];
            cDist[j][i] = distMatrix[i][j];
        }
    }

    // Agglomerative merging
    int activeCount = n;
    int nextClusterId = n;

    while (activeCount > 1) {
        // Find closest pair
        double minDist = 1e300;
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < nextClusterId; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < nextClusterId; ++j) {
                if (!active[j]) continue;
                if (cDist[i][j] < minDist) {
                    minDist = cDist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;

        // Check threshold
        if (m_threshold > 0 && minDist > m_threshold && activeCount <= m_k)
            break;

        // Record merge
        MergeStep step;
        step.clusterA = bestI;
        step.clusterB = bestJ;
        step.distance = minDist;
        step.newSize = members[bestI].size() + members[bestJ].size();
        result.merges.append(step);

        // Create merged cluster
        int newId = nextClusterId++;
        active.resize(newId + 1);
        active.append(false);
        members.resize(newId + 1);
        centroids.resize(newId + 1);
        cDist.resize(newId + 1);
        for (auto& row : cDist) row.resize(newId + 1, 1e300);

        members[newId] = members[bestI];
        members[newId].append(members[bestJ]);
        centroids[newId] = computeCentroid(data, members[newId]);

        // Update distances (centroid linkage)
        active[newId] = true;
        for (int k = 0; k < newId; ++k) {
            if (!active[k]) continue;
            double d = euclideanDistance(centroids[newId], centroids[k]);
            cDist[newId][k] = d;
            cDist[k][newId] = d;
        }
        cDist[newId][newId] = 0.0;

        active[bestI] = false;
        active[bestJ] = false;
        activeCount--;
    }

    // Assign cluster labels
    result.assignments = cutDendrogram(result.merges, n, m_k);
    result.numClusters = m_k;
    result.copheneticCorrelation = copheneticCorrelation(distMatrix, result.merges);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.totalFits++;
    m_cophSum += result.copheneticCorrelation;
    m_stats.avgCophenetic = m_cophSum / m_stats.totalFits;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(n, m_k, result.copheneticCorrelation, elapsed);
    return result;
}

/* ---- Reset ---- */

void HierarchicalCluster15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_cophSum = 0.0;
}
