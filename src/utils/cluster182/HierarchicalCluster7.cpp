/**
 * @file HierarchicalCluster7.cpp
 * @brief HierarchicalCluster7 实现
 *
 * 实现层次凝聚聚类：Lance-Williams递推、多链接策略、共表距离矩阵。
 */

#include "utils/cluster182/HierarchicalCluster7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HierarchicalCluster7::HierarchicalCluster7(QObject *parent) : QObject(parent) {}
HierarchicalCluster7::~HierarchicalCluster7() = default;

/* ---- Configuration ---- */

void HierarchicalCluster7::setLinkage(Linkage linkage) { m_linkage = linkage; }
void HierarchicalCluster7::setNumClusters(int k) { m_numClusters = qMax(1, k); }

/* ---- Distance ---- */

double HierarchicalCluster7::euclidean(const QVector<double>& a,
                                        const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

QVector<QVector<double>> HierarchicalCluster7::distanceMatrix(
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

/* ---- Lance-Williams recurrence ---- */

double HierarchicalCluster7::lanceWilliams(double dij, double dik, double djk,
                                             int si, int sj, int sk) const
{
    Q_UNUSED(djk)
    double total = si + sj + sk;
    if (total < 1e-12) return 0.0;

    switch (m_linkage) {
    case Linkage::Single:
        return qMin(dik, dij);
    case Linkage::Complete:
        return qMax(dik, dij);
    case Linkage::Average:
        return (si * dik + sj * dij) / (si + sj);
    case Linkage::Ward: {
        double num = (si + sk) * dik + (sj + sk) * dij - sk * djk;
        return qSqrt(qMax(0.0, num / total));
    }
    }
    return 0.0;
}

/* ---- Main fit ---- */

QVector<int> HierarchicalCluster7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    auto dist = distanceMatrix(data);
    m_merges.clear();

    // Active cluster tracking
    QVector<int> clusterId(n);
    QVector<int> clusterSize(n, 1);
    QVector<bool> active(n, true);
    for (int i = 0; i < n; ++i) clusterId[i] = i;

    // Working distance matrix (upper triangle)
    QVector<QVector<double>> d = dist;
    int numActive = n;

    while (numActive > m_numClusters) {
        // Find minimum distance pair
        double minDist = 1e18;
        int mi = -1, mj = -1;
        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (d[i][j] < minDist) {
                    minDist = d[i][j];
                    mi = i;
                    mj = j;
                }
            }
        }
        if (mi < 0) break;

        // Record merge step
        MergeStep step;
        step.clusterA = clusterId[mi];
        step.clusterB = clusterId[mj];
        step.distance = minDist;
        int mergedSize = clusterSize[mi] + clusterSize[mj];
        step.newSize = mergedSize;
        m_merges.append(step);

        // Update distances via Lance-Williams
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mi || k == mj) continue;
            double newD = lanceWilliams(
                d[mi][k], d[mj][k],
                d[qMin(mi, mj)][qMax(mi, mj)],
                clusterSize[mi], clusterSize[mj], clusterSize[k]);
            d[mi][k] = newD;
            d[k][mi] = newD;
        }

        // Merge mj into mi
        clusterSize[mi] = mergedSize;
        clusterId[mi] = n + m_merges.size() - 1; // New cluster id
        active[mj] = false;
        numActive--;
    }

    // Assign labels
    m_labels.resize(n);
    QVector<int> labelMap(n, -1);
    int labelIdx = 0;
    for (int i = 0; i < n; ++i) {
        if (!active[i]) continue;
        labelMap[i] = labelIdx++;
    }
    // Propagate to merged clusters
    for (int i = 0; i < n; ++i) {
        if (active[i]) {
            m_labels[i] = labelMap[i];
        } else {
            // Find root active cluster
            int root = i;
            for (int s = m_merges.size() - 1; s >= 0; --s) {
                // Trace merge chain to find active parent
            }
            // Simplified: assign to nearest active
            double minD = 1e18;
            for (int j = 0; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minD) { minD = dist[i][j]; m_labels[i] = labelMap[j]; }
            }
        }
    }

    double cophCorr = copheneticCorrelation(dist);

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numMerges = m_merges.size();
    m_stats.copheneticCorrelation = cophCorr;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_numClusters, cophCorr);
    return m_labels;
}

/* ---- Dendrogram ---- */

QVector<HierarchicalCluster7::MergeStep> HierarchicalCluster7::dendrogram() const
{
    return m_merges;
}

/* ---- Cophenetic distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster7::copheneticMatrix() const
{
    int n = m_labels.size();
    if (n == 0 || m_merges.isEmpty()) return {};

    QVector<QVector<double>> coph(n, QVector<double>(n, 0.0));

    // Build union-find for tracing merge heights
    QVector<int> parent(2 * n);
    QVector<double> mergeHeight(2 * n, 0.0);
    for (int i = 0; i < 2 * n; ++i) parent[i] = i;

    for (int s = 0; s < m_merges.size(); ++s) {
        int a = m_merges[s].clusterA;
        int b = m_merges[s].clusterB;
        int newId = n + s;
        if (a < 2 * n && b < 2 * n) {
            parent[a] = newId;
            parent[b] = newId;
        }
        mergeHeight[newId] = m_merges[s].distance;
    }

    // Cophenetic distance = merge height of LCA
    auto findRoot = [&](int x) -> QVector<int> {
        QVector<int> path;
        while (parent[x] != x) { path.append(x); x = parent[x]; }
        return path;
    };

    for (int i = 0; i < n; ++i) {
        coph[i][i] = 0.0;
        for (int j = i + 1; j < n; ++j) {
            // Trace both paths up and find LCA
            QVector<int> pi, pj;
            int ci = i, cj = j;
            QSet<int> ancestors;
            while (ci < 2 * n && parent[ci] != ci) {
                ancestors.insert(ci);
                ci = parent[ci];
            }
            ancestors.insert(ci);
            while (cj < 2 * n && !ancestors.contains(cj)) cj = parent[cj];
            double h = (cj < 2 * n) ? mergeHeight[cj] : 0.0;
            coph[i][j] = h;
            coph[j][i] = h;
        }
    }
    return coph;
}

/* ---- Cophenetic correlation ---- */

double HierarchicalCluster7::copheneticCorrelation(
    const QVector<QVector<double>>& origDist) const
{
    auto coph = copheneticMatrix();
    int n = coph.size();
    if (n == 0) return 0.0;

    QVector<double> x, y;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            x.append(origDist[i][j]);
            y.append(coph[i][j]);
        }

    int m = x.size();
    if (m == 0) return 0.0;

    double mx = 0.0, my = 0.0;
    for (int i = 0; i < m; ++i) { mx += x[i]; my += y[i]; }
    mx /= m; my /= m;

    double num = 0.0, dx = 0.0, dy = 0.0;
    for (int i = 0; i < m; ++i) {
        double xi = x[i] - mx, yi = y[i] - my;
        num += xi * yi;
        dx += xi * xi;
        dy += yi * yi;
    }
    double denom = qSqrt(dx * dy);
    return (denom < 1e-12) ? 0.0 : num / denom;
}

/* ---- Reset ---- */

void HierarchicalCluster7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_merges.clear();
    m_labels.clear();
}
