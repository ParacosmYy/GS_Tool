/**
 * @file HierarchicalCluster14.cpp
 * @brief HierarchicalCluster14 实现
 *
 * 实现层次聚类：非加权配对组法与共表距离矩阵的树状图验证。
 */

#include "utils/cluster279/HierarchicalCluster14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HierarchicalCluster14::HierarchicalCluster14(QObject *parent)
    : QObject(parent) {}

HierarchicalCluster14::~HierarchicalCluster14() = default;

/* ---- Configuration ---- */

void HierarchicalCluster14::setNumClusters(int k) { m_k = qBound(2, k, 200); }

/* ---- Euclidean distance ---- */

double HierarchicalCluster14::euclidean(const QVector<double>& a,
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

/* ---- Build pairwise distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster14::buildDistMatrix(
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

/* ---- UPGMA linkage distance between two clusters ---- */

double HierarchicalCluster14::upgmaDistance(const QVector<QVector<double>>& dist,
                                             const QVector<int>& ci,
                                             const QVector<int>& cj) const
{
    // Unweighted pair group method: average of all pairwise distances
    double sum = 0.0;
    int count = 0;
    for (int a : ci) {
        for (int b : cj) {
            if (a < dist.size() && b < dist[a].size())
                sum += dist[a][b];
            count++;
        }
    }
    return (count > 0) ? sum / count : 0.0;
}

/* ---- Agglomerative clustering with UPGMA ---- */

HierarchicalCluster14::ClusterResult HierarchicalCluster14::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < 2) return result;

    auto dist = buildDistMatrix(data);

    // Track active clusters: cluster label -> member point indices
    QVector<QVector<int>> clusters(n);
    QVector<bool> active(n, true);
    for (int i = 0; i < n; ++i) clusters[i].append(i);

    // Distance cache between cluster pairs
    QVector<QVector<double>> cdist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            cdist[i][j] = dist[i][j];
            cdist[j][i] = dist[i][j];
        }

    int nextLabel = n;
    int activeCount = n;

    // Iteratively merge closest pair
    for (int step = 0; step < n - 1; ++step) {
        // Find closest active pair
        double minDist = 1e30;
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < nextLabel; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < nextLabel; ++j) {
                if (!active[j]) continue;
                if (cdist[i][j] < minDist) {
                    minDist = cdist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;

        // Record merge step
        MergeStep merge;
        merge.clusterA = bestI;
        merge.clusterB = bestJ;
        merge.distance = minDist;
        merge.newSize = clusters[bestI].size() + clusters[bestJ].size();
        result.merges.append(merge);

        emit mergeDone(step, bestI, bestJ, minDist);

        // Create merged cluster
        clusters.append(clusters[bestI] + clusters[bestJ]);
        active.append(true);
        active[bestI] = false;
        active[bestJ] = false;

        // Compute UPGMA distances from new cluster to all remaining active clusters
        cdist.append(QVector<double>(nextLabel + 1, 0.0));
        for (int k = 0; k < nextLabel; ++k) {
            if (!active[k]) continue;
            double d = upgmaDistance(dist, clusters[bestI], clusters[k]);
            // UPGMA weighted average
            int ni = clusters[bestI].size();
            int nj = clusters[bestJ].size();
            double d2 = cdist[bestJ][k];
            double avg = (ni * d + nj * d2) / (ni + nj);
            cdist[nextLabel][k] = avg;
            cdist[k][nextLabel] = avg;
        }
        // Ensure square
        for (auto& row : cdist) row.resize(nextLabel + 1);
        cdist[nextLabel].resize(nextLabel + 1);

        nextLabel++;
        activeCount--;
    }

    // Compute cophenetic matrix and correlation
    auto cophDist = copheneticMatrix(result.merges, n);
    result.copheneticCorrelation = copheneticCorrelation(dist, cophDist);
    result.clusters = clusters;

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(n, m_k, result.copheneticCorrelation, elapsed);

    return result;
}

/* ---- Cophenetic distance matrix from dendrogram ---- */

QVector<QVector<double>> HierarchicalCluster14::copheneticMatrix(
    const QVector<MergeStep>& merges, int n) const
{
    // Union-Find with merge height tracking
    QVector<int> parent(2 * n);
    QVector<double> mergeHeight(2 * n, 0.0);
    for (int i = 0; i < 2 * n; ++i) parent[i] = i;

    auto find = [&](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    int nextLabel = n;
    for (const auto& m : merges) {
        parent[m.clusterA] = nextLabel;
        parent[m.clusterB] = nextLabel;
        mergeHeight[nextLabel] = m.distance;
        nextLabel++;
    }

    // For each pair, cophenetic distance = merge height of their LCA
    QVector<QVector<double>> coph(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            // Walk up from i and j to find meeting point
            int a = i, b = j;
            QSet<int> pathA;
            while (parent[a] != a) { pathA.insert(a); a = parent[a]; }
            pathA.insert(a);
            // Walk from b until hitting pathA
            int bCur = b;
            double height = 0.0;
            while (!pathA.contains(bCur)) {
                bCur = parent[bCur];
            }
            // bCur is LCA; find merge height by walking from i up
            a = i;
            while (parent[a] != a && parent[a] != bCur) {
                height = qMax(height, mergeHeight[parent[a]]);
                a = parent[a];
            }
            if (parent[a] == bCur) height = mergeHeight[bCur];
            coph[i][j] = height;
            coph[j][i] = height;
        }
    }
    return coph;
}

/* ---- Cophenetic correlation coefficient ---- */

double HierarchicalCluster14::copheneticCorrelation(
    const QVector<QVector<double>>& originalDist,
    const QVector<QVector<double>>& cophDist) const
{
    int n = qMin(originalDist.size(), cophDist.size());
    if (n < 2) return 0.0;

    // Pearson correlation between upper triangle of original and cophenetic
    double sumXY = 0.0, sumX = 0.0, sumY = 0.0;
    double sumX2 = 0.0, sumY2 = 0.0;
    int count = 0;

    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double x = originalDist[i][j];
            double y = cophDist[i][j];
            sumX += x; sumY += y;
            sumX2 += x * x; sumY2 += y * y;
            sumXY += x * y;
            count++;
        }

    if (count == 0) return 0.0;
    double num = sumXY - sumX * sumY / count;
    double denX = sumX2 - sumX * sumX / count;
    double denY = sumY2 - sumY * sumY / count;
    double den = qSqrt(denX * denY);
    return (den > 1e-15) ? num / den : 0.0;
}

/* ---- Cut dendrogram at distance threshold ---- */

QVector<int> HierarchicalCluster14::cutAtDistance(const QVector<MergeStep>& merges,
                                                    int n, double threshold) const
{
    QVector<int> label(2 * n, -1);
    for (int i = 0; i < n; ++i) label[i] = i;

    auto findLabel = [&](int x) -> int {
        while (label[x] != x && label[x] >= 0) x = label[x];
        return x;
    };

    int nextLabel = n;
    for (const auto& m : merges) {
        if (m.distance > threshold) break;
        label[m.clusterA] = nextLabel;
        label[m.clusterB] = nextLabel;
        label.append(nextLabel);
        nextLabel++;
    }

    // Assign cluster ids
    QVector<int> result(n);
    QMap<int, int> labelMap;
    int cid = 0;
    for (int i = 0; i < n; ++i) {
        int root = findLabel(i);
        if (!labelMap.contains(root)) labelMap[root] = cid++;
        result[i] = labelMap[root];
    }
    return result;
}

/* ---- Get labels for k clusters ---- */

QVector<int> HierarchicalCluster14::getLabels(const QVector<MergeStep>& merges,
                                                int n, int k) const
{
    if (n <= 0) return {};
    if (k >= n) {
        QVector<int> labels(n);
        for (int i = 0; i < n; ++i) labels[i] = i;
        return labels;
    }
    // Stop at merge step (n - k) to get k clusters
    int stopAt = n - k;
    double threshold = (stopAt > 0 && stopAt <= merges.size())
                           ? merges[stopAt - 1].distance : 0.0;
    if (stopAt > 0 && stopAt <= merges.size())
        return cutAtDistance(merges, n, merges[stopAt - 1].distance * 0.999);
    return cutAtDistance(merges, n, 1e30);
}

/* ---- Reset ---- */

void HierarchicalCluster14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
