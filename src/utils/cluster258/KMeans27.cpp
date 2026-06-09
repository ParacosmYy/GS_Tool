/**
 * @file KMeans27.cpp
 * @brief KMeans27 实现
 *
 * 实现K均值二分聚类：SSE分裂准则与层次分裂式聚类。
 */

#include "utils/cluster258/KMeans27.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

KMeans27::KMeans27(QObject *parent)
    : QObject(parent) {}
KMeans27::~KMeans27() = default;

/* ---- Configuration ---- */

void KMeans27::setNumClusters(int k) { m_k = qMax(2, k); }
void KMeans27::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }

/* ---- Distance & centroid helpers ---- */

double KMeans27::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

QVector<double> KMeans27::computeCentroid(const QVector<int>& indices) const
{
    int n = indices.size();
    QVector<double> cen(m_dims, 0.0);
    for (int idx : indices) {
        for (int d = 0; d < m_dims; ++d)
            cen[d] += m_data[idx][d];
    }
    for (int d = 0; d < m_dims; ++d)
        cen[d] /= qMax(n, 1);
    return cen;
}

double KMeans27::computeSSE(const QVector<int>& indices, const QVector<double>& centroid) const
{
    double sse = 0.0;
    for (int idx : indices) {
        double d = distSq(m_data[idx], centroid);
        sse += d;
    }
    return sse;
}

/* ---- Bisect: basic 2-means on a subset ---- */

QVector<KMeans27::Cluster> KMeans27::bisect(const QVector<int>& indices) const
{
    int n = indices.size();
    if (n < 2) {
        Cluster c;
        c.centroid = (n == 1) ? m_data[indices[0]] : QVector<double>(m_dims, 0.0);
        c.memberIndices = indices;
        c.sse = 0.0;
        return {c};
    }

    // Initialize two centroids using farthest point
    std::mt19937 rng(42);
    int i0 = indices[rng() % n];
    int farIdx = indices[0];
    double maxD = 0.0;
    for (int idx : indices) {
        double d = distSq(m_data[idx], m_data[i0]);
        if (d > maxD) { maxD = d; farIdx = idx; }
    }

    QVector<double> cenA = m_data[i0];
    QVector<double> cenB = m_data[farIdx];
    QVector<int> assign(n, 0);

    // Iterate K-means with 2 clusters
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Assignment step
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double dA = distSq(m_data[indices[i]], cenA);
            double dB = distSq(m_data[indices[i]], cenB);
            int newA = (dA <= dB) ? 0 : 1;
            if (newA != assign[i]) { assign[i] = newA; changed = true; }
        }
        if (!changed) break;

        // Update step
        QVector<double> sumA(m_dims, 0.0), sumB(m_dims, 0.0);
        int cntA = 0, cntB = 0;
        for (int i = 0; i < n; ++i) {
            if (assign[i] == 0) {
                for (int d = 0; d < m_dims; ++d) sumA[d] += m_data[indices[i]][d];
                cntA++;
            } else {
                for (int d = 0; d < m_dims; ++d) sumB[d] += m_data[indices[i]][d];
                cntB++;
            }
        }
        if (cntA > 0) for (int d = 0; d < m_dims; ++d) cenA[d] = sumA[d] / cntA;
        if (cntB > 0) for (int d = 0; d < m_dims; ++d) cenB[d] = sumB[d] / cntB;
    }

    // Build two clusters
    QVector<Cluster> result(2);
    result[0].centroid = cenA;
    result[1].centroid = cenB;
    for (int i = 0; i < n; ++i)
        result[assign[i]].memberIndices.append(indices[i]);
    result[0].sse = computeSSE(result[0].memberIndices, cenA);
    result[1].sse = computeSSE(result[1].memberIndices, cenB);
    return result;
}

/* ---- Find cluster with max SSE ---- */

int KMeans27::findMaxSSECluster() const
{
    int best = 0;
    for (int i = 1; i < m_clusters.size(); ++i)
        if (m_clusters[i].sse > m_clusters[best].sse) best = i;
    return best;
}

/* ---- Fit ---- */

bool KMeans27::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_k) return false;
    m_data = data;
    m_dims = data[0].size();

    // Start with one cluster containing all points
    m_clusters.clear();
    Cluster root;
    root.memberIndices.resize(n);
    for (int i = 0; i < n; ++i) root.memberIndices[i] = i;
    root.centroid = computeCentroid(root.memberIndices);
    root.sse = computeSSE(root.memberIndices, root.centroid);
    m_clusters.append(root);

    // Repeatedly bisect the cluster with highest SSE
    while (m_clusters.size() < m_k) {
        int splitIdx = findMaxSSECluster();
        double sseBefore = m_clusters[splitIdx].sse;
        QVector<int> members = m_clusters[splitIdx].memberIndices;
        QVector<Cluster> halves = bisect(members);

        // Replace the split cluster with the two halves
        m_clusters.removeAt(splitIdx);
        for (int i = halves.size() - 1; i >= 0; --i)
            m_clusters.insert(splitIdx, halves[i]);

        double sseAfter = 0.0;
        for (const auto& c : m_clusters) sseAfter += c.sse;
        emit clusterSplit(splitIdx, sseBefore, sseAfter);
    }

    double elapsed = timer.elapsed();
    m_stats.numClusters = m_clusters.size();
    m_stats.numSamples = n;
    m_stats.numDimensions = m_dims;
    m_stats.numSplits = m_k - 1;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(m_clusters.size(), totalSSE(), elapsed);
    return true;
}

/* ---- Predict ---- */

int KMeans27::predict(const QVector<double>& sample) const
{
    if (m_clusters.isEmpty()) return -1;
    int best = 0;
    double bestDist = distSq(sample, m_clusters[0].centroid);
    for (int i = 1; i < m_clusters.size(); ++i) {
        double d = distSq(sample, m_clusters[i].centroid);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

/* ---- Accessors ---- */

QVector<KMeans27::Cluster> KMeans27::clusters() const { return m_clusters; }

double KMeans27::totalSSE() const
{
    double sse = 0.0;
    for (const auto& c : m_clusters) sse += c.sse;
    return sse;
}

/* ---- Reset ---- */

void KMeans27::resetStatistics()
{
    m_clusters.clear();
    m_data.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
