/**
 * @file HierarchicalCluster8.cpp
 * @brief HierarchicalCluster8 实现
 *
 * 实现层次聚类：WPGMA/WPGMO加权、不一致系数链切割、树状图构建。
 */

#include "utils/cluster198/HierarchicalCluster8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HierarchicalCluster8::HierarchicalCluster8(QObject *parent) : QObject(parent) {}
HierarchicalCluster8::~HierarchicalCluster8() = default;

/* ---- Configuration ---- */

void HierarchicalCluster8::setNumClusters(int k) { m_k = qMax(2, k); }
void HierarchicalCluster8::setLinkage(Linkage method) { m_linkage = method; }
void HierarchicalCluster8::setInconsistentThreshold(double t) { m_inconsistentThreshold = qMax(0.0, t); }
void HierarchicalCluster8::setMaxInconsistentDepth(int depth) { m_maxDepth = qMax(1, depth); }

/* ---- Condensed index mapping ---- */

int HierarchicalCluster8::condensedIndex(int i, int j, int n) const
{
    if (i > j) qSwap(i, j);
    return n * i - i * (i + 1) / 2 + (j - i - 1);
}

/* ---- Find minimum pair ---- */

QPair<int, int> HierarchicalCluster8::findMinPair(const QVector<double>& condensed, int n) const
{
    double best = std::numeric_limits<double>::max();
    int bi = 0, bj = 1;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            int idx = condensedIndex(i, j, n);
            if (idx >= 0 && idx < condensed.size() && condensed[idx] < best) {
                best = condensed[idx]; bi = i; bj = j;
            }
        }
    return {bi, bj};
}

/* ---- WPGMA/WPGMO merged distance ---- */

double HierarchicalCluster8::mergedDistance(double dij, double dik, double djk,
                                              int si, int sj, int sk) const
{
    Q_UNUSED(dij)
    if (m_linkage == Linkage::WPGMA) {
        // WPGMA: (dik*si + djk*sj) / (si + sj)
        return (dik * si + djk * sj) / static_cast<double>(si + sj);
    } else if (m_linkage == Linkage::WPGMO) {
        // WPGMO: weighted pair-group median
        return (dik + djk) / 2.0;
    } else if (m_linkage == Linkage::Single) {
        return qMin(dik, djk);
    } else {
        return qMax(dik, djk);
    }
}

/* ---- Build dendrogram ---- */

QVector<QVector<double>> HierarchicalCluster8::buildDendrogram(const QVector<QVector<double>>& distMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = distMatrix.size();
    m_dendrogram.clear();

    // Active cluster tracking
    QVector<int> active;
    for (int i = 0; i < n; ++i) active.append(i);
    QVector<int> sizes(n, 1);

    // Copy distance matrix to mutable condensed form
    int condensedSize = n * (n - 1) / 2;
    QVector<double> dist(condensedSize, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            dist[condensedIndex(i, j, n)] = distMatrix[i][j];

    int nextLabel = n;

    for (int step = 0; step < n - 1; ++step) {
        // Find closest pair among active clusters
        double bestDist = std::numeric_limits<double>::max();
        int bi = 0, bj = 1;
        for (int a = 0; a < active.size(); ++a)
            for (int b = a + 1; b < active.size(); ++b) {
                int ci = active[a], cj = active[b];
                int idx = condensedIndex(ci, cj, n + step);
                if (idx >= 0 && idx < dist.size() && dist[idx] < bestDist) {
                    bestDist = dist[idx]; bi = a; bj = b;
                }
            }

        int ci = active[bi], cj = active[bj];
        int newSize = sizes[ci] + sizes[cj];

        // Record dendrogram row: [cluster_i, cluster_j, distance, new_size]
        QVector<double> row;
        row.append(ci); row.append(cj); row.append(bestDist); row.append(newSize);
        m_dendrogram.append(row);

        // Update distances using WPGMA/WPGMO
        int newLabel = nextLabel++;
        sizes.resize(newLabel + 1);
        sizes[newLabel] = newSize;

        // Expand condensed storage
        int newN = n + step + 1;
        dist.resize(newN * (newN - 1) / 2);

        for (int a = 0; a < active.size(); ++a) {
            if (a == bi || a == bj) continue;
            int ck = active[a];
            int idxIK = condensedIndex(qMin(ci, ck), qMax(ci, ck), newN);
            int idxJK = condensedIndex(qMin(cj, ck), qMax(cj, ck), newN);
            double dik = (idxIK >= 0 && idxIK < dist.size()) ? dist[idxIK] : 0.0;
            double djk = (idxJK >= 0 && idxJK < dist.size()) ? dist[idxJK] : 0.0;
            double dm = mergedDistance(bestDist, dik, djk, sizes[ci], sizes[cj], sizes[ck]);
            int newIdx = condensedIndex(qMin(newLabel, ck), qMax(newLabel, ck), newN);
            if (newIdx >= 0 && newIdx < dist.size()) dist[newIdx] = dm;
        }

        // Replace bi with merged cluster, remove bj
        active[bi] = newLabel;
        active.removeAt(bj);
    }

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = m_k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    return m_dendrogram;
}

/* ---- Cut tree into k clusters ---- */

QVector<int> HierarchicalCluster8::cutTree(const QVector<QVector<double>>& dendro, int k) const
{
    int n = dendro.size() + 1;
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) labels[i] = i;

    // Undo merges: stop when we have k clusters
    int numMerges = dendro.size() - k + 1;
    return findComponents(n, dendro, numMerges);
}

/* ---- Find connected components ---- */

QVector<int> HierarchicalCluster8::findComponents(int n, const QVector<QVector<double>>& dendro,
                                                      int numMerges) const
{
    // Union-Find
    QVector<int> parent(n + dendro.size());
    QVector<int> rank_(n + dendro.size(), 0);
    for (int i = 0; i < parent.size(); ++i) parent[i] = i;

    // Merge first numMerges steps
    for (int i = 0; i < numMerges && i < dendro.size(); ++i) {
        int a = static_cast<int>(dendro[i][0]);
        int b = static_cast<int>(dendro[i][1]);
        // Union a and b
        int rootA = a, rootB = b;
        while (parent[rootA] != rootA) { parent[rootA] = parent[parent[rootA]]; rootA = parent[rootA]; }
        while (parent[rootB] != rootB) { parent[rootB] = parent[parent[rootB]]; rootB = parent[rootB]; }
        if (rootA != rootB) {
            if (rank_[rootA] < rank_[rootB]) qSwap(rootA, rootB);
            parent[rootB] = rootA;
            if (rank_[rootA] == rank_[rootB]) rank_[rootA]++;
        }
    }

    // Assign cluster labels
    QVector<int> labels(n);
    QMap<int, int> labelMap;
    int nextLabel = 0;
    for (int i = 0; i < n; ++i) {
        int root = i;
        while (parent[root] != root) root = parent[root];
        if (!labelMap.contains(root)) labelMap[root] = nextLabel++;
        labels[i] = labelMap[root];
    }
    return labels;
}

/* ---- Compute inconsistency coefficients ---- */

QVector<double> HierarchicalCluster8::computeInconsistency(const QVector<QVector<double>>& dendro) const
{
    int m = dendro.size();
    QVector<double> incon(m, 0.0);

    for (int i = 0; i < m; ++i) {
        double height = dendro[i][2];
        int a = static_cast<int>(dendro[i][0]);
        int b = static_cast<int>(dendro[i][1]);

        // Collect heights of descendant links up to depth
        QVector<double> desc;
        desc.append(height);

        QVector<int> queue;
        queue.append(a); queue.append(b);
        int depth = 0;
        while (depth < m_maxDepth && !queue.isEmpty()) {
            QVector<int> next;
            for (int node : queue) {
                if (node < m) {
                    desc.append(dendro[node][2]);
                    next.append(static_cast<int>(dendro[node][0]));
                    next.append(static_cast<int>(dendro[node][1]));
                }
            }
            queue = next;
            depth++;
        }

        // Mean and std of descendant heights
        double mean = 0.0;
        for (double d : desc) mean += d;
        mean /= desc.size();

        double var = 0.0;
        for (double d : desc) var += (d - mean) * (d - mean);
        double stdDev = qSqrt(var / desc.size());

        incon[i] = (stdDev > 1e-12) ? (height - mean) / stdDev : 0.0;
    }
    return incon;
}

/* ---- Inconsistent-link cut ---- */

QVector<int> HierarchicalCluster8::cutInconsistent(const QVector<QVector<double>>& dendro) const
{
    int n = dendro.size() + 1;
    QVector<double> incon = computeInconsistency(dendro);

    // Find deepest link that exceeds threshold
    int numMerges = 0;
    for (int i = 0; i < dendro.size(); ++i) {
        if (incon[i] > m_inconsistentThreshold) { numMerges = i; break; }
        numMerges = i + 1;
    }

    return findComponents(n, dendro, numMerges);
}

/* ---- Fit ---- */

QVector<int> HierarchicalCluster8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    // Build distance matrix (Euclidean)
    QVector<QVector<double>> dm(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double sum = 0.0;
            int d = qMin(data[i].size(), data[j].size());
            for (int k = 0; k < d; ++k) { double diff = data[i][k] - data[j][k]; sum += diff * diff; }
            dm[i][j] = dm[j][i] = qSqrt(sum);
        }

    m_distMatrix = dm;
    buildDendrogram(dm);
    QVector<int> labels = cutTree(m_dendrogram, m_k);

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = m_k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit clusteringCompleted(m_k, n, timer.elapsed());
    return labels;
}

/* ---- Reset ---- */

void HierarchicalCluster8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dendrogram.clear();
    m_distMatrix.clear();
}
