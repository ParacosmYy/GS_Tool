/**
 * @file HierarchicalCluster9.cpp
 * @brief HierarchicalCluster9 实现
 *
 * 实现层次聚类：Lance-Williams柔性更新、共表相关验证、多种链接策略。
 */

#include "utils/cluster209/HierarchicalCluster9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

HierarchicalCluster9::HierarchicalCluster9(QObject *parent) : QObject(parent) {}
HierarchicalCluster9::~HierarchicalCluster9() = default;

/* ---- Configuration ---- */

void HierarchicalCluster9::setLinkage(Linkage method) { m_linkage = method; }
void HierarchicalCluster9::setDistanceMetric(int metric) { m_metric = metric; }

/* ---- Distance function ---- */

double HierarchicalCluster9::distance(const QVector<double>& a,
                                       const QVector<double>& b, int metric)
{
    if (metric == 1) {
        // Manhattan
        double sum = 0.0;
        int d = qMin(a.size(), b.size());
        for (int i = 0; i < d; ++i) sum += qAbs(a[i] - b[i]);
        return sum;
    }
    // Euclidean
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Condensed distance matrix ---- */

QVector<double> HierarchicalCluster9::condensedDistances(
    const QVector<QVector<double>>& data, int metric)
{
    int n = data.size();
    int size = n * (n - 1) / 2;
    QVector<double> dist(size, 0.0);
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[idx++] = distance(data[i], data[j], metric);
        }
    }
    return dist;
}

/* ---- Lance-Williams update ---- */

double HierarchicalCluster9::lanceWilliamsUpdate(double d_iq, double d_jq,
                                                   int si, int sj, int sq) const
{
    int n_i = si, n_j = sj, n_q = sq;
    double n_s = n_i + n_j;

    switch (m_linkage) {
    case SingleLink:
        return qMin(d_iq, d_jq);
    case CompleteLink:
        return qMax(d_iq, d_jq);
    case AverageLink: {
        // UPGMA: weighted by cluster sizes
        double ai = n_i / n_s;
        double aj = n_j / n_s;
        return ai * d_iq + aj * d_jq;
    }
    case WardLink: {
        // Ward's minimum variance
        double ai = (n_i + n_q) / (n_s + n_q);
        double aj = (n_j + n_q) / (n_s + n_q);
        double beta = -n_q / (n_s + n_q);
        return ai * d_iq + aj * d_jq + beta * qAbs(d_iq - d_jq);
    }
    }
    return (d_iq + d_jq) / 2.0;
}

/* ---- Fit ---- */

void HierarchicalCluster9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n <= 1) return;

    // Condensed distance matrix
    auto dist = condensedDistances(data, m_metric);

    // Active clusters: each point starts as its own cluster
    QVector<int> clusterSize(m_n, 1);
    QVector<int> clusterId(m_n);
    for (int i = 0; i < m_n; ++i) clusterId[i] = i;

    // Map from cluster index to position in active set
    QVector<bool> active(m_n, true);
    QVector<int> membership(m_n);
    for (int i = 0; i < m_n; ++i) membership[i] = i;

    // Distance matrix indexed by pair (i,j) where i<j
    // Helper: index into condensed matrix
    auto idx = [this](int i, int j) -> int {
        if (i > j) std::swap(i, j);
        return i * m_n - i * (i + 1) / 2 + j - i - 1;
    };

    m_merges.clear();
    int nextCluster = m_n;

    for (int step = 0; step < m_n - 1; ++step) {
        // Find minimum distance among active clusters
        double minDist = std::numeric_limits<double>::max();
        int mergeI = -1, mergeJ = -1;

        for (int i = 0; i < m_n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < m_n; ++j) {
                if (!active[j]) continue;
                int dIdx = idx(i, j);
                if (dIdx >= 0 && dIdx < dist.size() && dist[dIdx] < minDist) {
                    minDist = dist[dIdx];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        // Record merge event
        MergeEvent ev;
        ev.clusterA = clusterId[mergeI];
        ev.clusterB = clusterId[mergeJ];
        ev.distance = minDist;
        ev.newSize = clusterSize[mergeI] + clusterSize[mergeJ];
        m_merges.append(ev);

        // Update distances using Lance-Williams formula
        int newSize = ev.newSize;
        for (int k = 0; k < m_n; ++k) {
            if (!active[k] || k == mergeI || k == mergeJ) continue;
            int dIK = idx(mergeI, k);
            int dJK = idx(mergeJ, k);
            double d_iq = (dIK >= 0 && dIK < dist.size()) ? dist[dIK] : 0.0;
            double d_jq = (dJK >= 0 && dJK < dist.size()) ? dist[dJK] : 0.0;
            double newDist = lanceWilliamsUpdate(d_iq, d_jq,
                                                  clusterSize[mergeI],
                                                  clusterSize[mergeJ],
                                                  clusterSize[k]);
            dist[dIK] = newDist;
        }

        // Merge: deactivate mergeJ, keep mergeI as merged cluster
        active[mergeJ] = false;
        clusterSize[mergeI] = newSize;
        clusterId[mergeI] = nextCluster++;
        for (int p = 0; p < m_n; ++p) {
            if (membership[p] == mergeJ) membership[p] = mergeI;
        }
    }

    m_labels = membership;

    m_stats.numPoints = m_n;
    m_stats.numDimensions = (m_n > 0) ? data[0].size() : 0;
    m_stats.numMerges = m_merges.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(m_merges.size(), 0.0, timer.elapsed());
}

/* ---- Cut tree at k clusters ---- */

QVector<int> HierarchicalCluster9::cutTree(int k) const
{
    if (m_n == 0 || m_merges.isEmpty()) return m_labels;
    k = qBound(1, k, m_n);

    // Union-Find to reconstruct from top
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    // Apply first (n-k) merges
    int mergeCount = m_n - k;
    for (int i = 0; i < mergeCount && i < m_merges.size(); ++i) {
        int a = m_merges[i].clusterA;
        int b = m_merges[i].clusterB;
        // Find root representatives
        if (a < m_n && b < m_n) parent[b] = a;
    }

    // Compress and assign labels
    QVector<int> labels(m_n);
    QVector<int> labelMap(m_n, -1);
    int nextLabel = 0;
    for (int i = 0; i < m_n; ++i) {
        int root = i;
        while (root < m_n && parent[root] != root) root = parent[root];
        if (labelMap[root] < 0) labelMap[root] = nextLabel++;
        labels[i] = labelMap[root];
    }
    return labels;
}

/* ---- Cut at distance threshold ---- */

QVector<int> HierarchicalCluster9::cutAtDistance(double threshold) const
{
    if (m_n == 0) return {};
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    for (const auto& ev : m_merges) {
        if (ev.distance > threshold) break;
        if (ev.clusterA < m_n && ev.clusterB < m_n)
            parent[ev.clusterB] = ev.clusterA;
    }

    QVector<int> labels(m_n);
    QVector<int> labelMap(m_n, -1);
    int nextLabel = 0;
    for (int i = 0; i < m_n; ++i) {
        int root = i;
        while (root < m_n && parent[root] != root) root = parent[root];
        if (labelMap[root] < 0) labelMap[root] = nextLabel++;
        labels[i] = labelMap[root];
    }
    return labels;
}

/* ---- Cophenetic distances ---- */

QVector<double> HierarchicalCluster9::copheneticDistances() const
{
    int n = m_n;
    int size = n * (n - 1) / 2;
    QVector<double> coph(size, 0.0);

    // For each pair, find the merge distance
    QVector<int> root(n);
    for (int i = 0; i < n; ++i) root[i] = i;

    // Track cluster membership through merges
    for (const auto& ev : m_merges) {
        int a = ev.clusterA, b = ev.clusterB;
        // Set cophenetic distance for pairs spanning A and B
        QVector<int> membersA, membersB;
        for (int i = 0; i < n; ++i) {
            if (root[i] == a) membersA.append(i);
            else if (root[i] == b) membersB.append(i);
        }
        for (int i : membersA) {
            for (int j : membersB) {
                int ii = qMin(i, j), jj = qMax(i, j);
                int idx = ii * n - ii * (ii + 1) / 2 + jj - ii - 1;
                if (idx >= 0 && idx < size) coph[idx] = ev.distance;
            }
        }
        // Merge: assign all B members to A
        int mergedId = a;
        for (int i = 0; i < n; ++i) {
            if (root[i] == a || root[i] == b) root[i] = mergedId;
        }
    }
    return coph;
}

/* ---- Cophenetic correlation ---- */

double HierarchicalCluster9::copheneticCorrelation(
    const QVector<QVector<double>>& data) const
{
    if (m_n <= 1) return 0.0;

    auto origDist = condensedDistances(data, m_metric);
    auto cophDist = copheneticDistances();

    int m = origDist.size();
    if (m == 0) return 0.0;

    // Pearson correlation between origDist and cophDist
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
    for (int i = 0; i < m; ++i) {
        sumX += origDist[i];
        sumY += cophDist[i];
        sumXY += origDist[i] * cophDist[i];
        sumX2 += origDist[i] * origDist[i];
        sumY2 += cophDist[i] * cophDist[i];
    }

    double num = m * sumXY - sumX * sumY;
    double den = qSqrt((m * sumX2 - sumX * sumX) * (m * sumY2 - sumY * sumY));
    double corr = (den > 0) ? num / den : 0.0;

    m_stats.copheneticCorrelation = corr;
    return corr;
}

/* ---- Getters ---- */

QVector<HierarchicalCluster9::MergeEvent> HierarchicalCluster9::dendrogram() const
{
    return m_merges;
}

QVector<int> HierarchicalCluster9::labels() const { return m_labels; }

/* ---- Reset ---- */

void HierarchicalCluster9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_merges.clear();
    m_labels.clear();
    m_n = 0;
}
