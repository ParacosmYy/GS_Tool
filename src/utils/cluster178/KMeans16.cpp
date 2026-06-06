/**
 * @file KMeans16.cpp
 * @brief KMeans16 实现
 *
 * 实现K均值聚类：k-means++初始化、小批量变体、二分K层次优化、轮廓系数。
 */

#include "utils/cluster178/KMeans16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMeans16::KMeans16(QObject *parent) : QObject(parent) {}
KMeans16::~KMeans16() = default;

/* ---- Configuration ---- */

void KMeans16::setNumClusters(int k) { m_numClusters = qMax(2, k); }
void KMeans16::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void KMeans16::setMode(Mode mode) { m_mode = mode; }
void KMeans16::setBatchSize(int size) { m_batchSize = qMax(4, size); }
void KMeans16::setTolerance(double tol) { m_tol = qMax(1e-10, tol); }
void KMeans16::setSeed(int seed) { m_seed = seed; }

/* ---- Distance utility ---- */

double KMeans16::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/* ---- k-means++ initialization ---- */

QVector<QVector<double>> KMeans16::kmeansPPInit(
    const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    QVector<QVector<double>> ctrs;
    ctrs.reserve(k);

    // First center: random (using seed)
    int first = m_seed % n;
    ctrs.append(data[first]);

    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < k; ++c) {
        // Update minimum distances to nearest existing center
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = distSq(data[i], ctrs.last());
            if (d < minDist[i]) minDist[i] = d;
            totalDist += minDist[i];
        }

        // Weighted random selection (proportional to distance squared)
        double threshold = (totalDist * ((m_seed * (c + 1)) % 10000)) / 10000.0;
        double cumDist = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumDist += minDist[i];
            if (cumDist >= threshold) { chosen = i; break; }
        }
        ctrs.append(data[chosen]);
    }
    return ctrs;
}

/* ---- Standard Lloyd's iteration ---- */

QVector<int> KMeans16::lloydsIterate(const QVector<QVector<double>>& data)
{
    auto initCenters = kmeansPPInit(data, m_numClusters);
    return singleKMeans(data, initCenters);
}

/* ---- Single K-means with given initial centers ---- */

QVector<int> KMeans16::singleKMeans(const QVector<QVector<double>>& data,
                                      QVector<QVector<double>>& ctrs)
{
    int n = data.size();
    int k = ctrs.size();
    QVector<int> labels(n, 0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;
        // Assign each point to nearest center
        for (int i = 0; i < n; ++i) {
            double bestD = 1e30;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = distSq(data[i], ctrs[c]);
                if (d < bestD) { bestD = d; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        // Recompute centers
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(m_dims, 0.0);
            int cnt = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int d = 0; d < m_dims; ++d) sum[d] += data[i][d];
                    ++cnt;
                }
            }
            if (cnt > 0) {
                for (int d = 0; d < m_dims; ++d) ctrs[c][d] = sum[d] / cnt;
            }
        }
    }
    m_centers = ctrs;
    return labels;
}

/* ---- Mini-batch K-means ---- */

QVector<int> KMeans16::miniBatchFit(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_numClusters;
    auto ctrs = kmeansPPInit(data, k);
    QVector<int> counts(k, 0);
    QVector<int> labels(n, 0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Sample mini-batch
        int bSize = qMin(m_batchSize, n);
        QVector<int> batch(bSize);
        for (int i = 0; i < bSize; ++i)
            batch[i] = (m_seed * (iter + 1) * (i + 1)) % n;

        // Assign batch points
        QVector<int> batchLabels(bSize);
        for (int i = 0; i < bSize; ++i) {
            double bestD = 1e30;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = distSq(data[batch[i]], ctrs[c]);
                if (d < bestD) { bestD = d; bestC = c; }
            }
            batchLabels[i] = bestC;
        }

        // Update centers incrementally
        for (int i = 0; i < bSize; ++i) {
            int c = batchLabels[i];
            counts[c]++;
            double eta = 1.0 / counts[c];
            for (int d = 0; d < m_dims; ++d)
                ctrs[c][d] = (1.0 - eta) * ctrs[c][d] + eta * data[batch[i]][d];
        }
    }

    // Final assignment
    for (int i = 0; i < n; ++i) {
        double bestD = 1e30;
        for (int c = 0; c < k; ++c) {
            double d = distSq(data[i], ctrs[c]);
            if (d < bestD) { bestD = d; labels[i] = c; }
        }
    }
    m_centers = ctrs;
    return labels;
}

/* ---- Bisecting K-means ---- */

QVector<int> KMeans16::bisectingFit(const QVector<QVector<double>>& data)
{
    int n = data.size();
    QVector<int> labels(n, 0);

    // Start with one cluster containing all points
    struct ClusterInfo {
        QVector<int> indices;
        double sse = 0.0;
    };
    QVector<ClusterInfo> clusters(1);
    for (int i = 0; i < n; ++i) clusters[0].indices.append(i);

    // Repeatedly bisect the cluster with highest SSE
    while (clusters.size() < m_numClusters) {
        // Find cluster with max SSE
        int worstIdx = 0;
        double maxSSE = 0.0;
        for (int c = 0; c < clusters.size(); ++c) {
            double sse = 0.0;
            // Compute centroid
            QVector<double> centroid(m_dims, 0.0);
            for (int idx : clusters[c].indices)
                for (int d = 0; d < m_dims; ++d) centroid[d] += data[idx][d];
            for (int d = 0; d < m_dims; ++d) centroid[d] /= clusters[c].indices.size();
            for (int idx : clusters[c].indices)
                sse += distSq(data[idx], centroid);
            clusters[c].sse = sse;
            if (sse > maxSSE) { maxSSE = sse; worstIdx = c; }
        }

        // Bisect worst cluster via 2-means
        ClusterInfo sub;
        sub.indices = clusters[worstIdx].indices;
        QVector<QVector<double>> subData;
        for (int idx : sub.indices) subData.append(data[idx]);

        auto subInit = kmeansPPInit(subData, 2);
        auto subLabels = singleKMeans(subData, subInit);

        ClusterInfo c0, c1;
        for (int i = 0; i < subLabels.size(); ++i) {
            if (subLabels[i] == 0) c0.indices.append(sub.indices[i]);
            else c1.indices.append(sub.indices[i]);
        }
        clusters[worstIdx] = c0;
        clusters.append(c1);
    }

    // Assign final labels
    for (int c = 0; c < clusters.size(); ++c)
        for (int idx : clusters[c].indices)
            labels[idx] = c;

    // Recompute centers
    m_centers.resize(clusters.size());
    for (int c = 0; c < clusters.size(); ++c) {
        m_centers[c].resize(m_dims, 0.0);
        for (int idx : clusters[c].indices)
            for (int d = 0; d < m_dims; ++d) m_centers[c][d] += data[idx][d];
        if (!clusters[c].indices.isEmpty())
            for (int d = 0; d < m_dims; ++d) m_centers[c][d] /= clusters[c].indices.size();
    }
    return labels;
}

/* ---- Inertia computation ---- */

double KMeans16::computeInertia(const QVector<QVector<double>>& data) const
{
    double inertia = 0.0;
    for (int i = 0; i < data.size(); ++i)
        inertia += distSq(data[i], m_centers[m_labels[i]]);
    return inertia;
}

/* ---- Silhouette score ---- */

double KMeans16::silhouetteScore(const QVector<QVector<double>>& data,
                                   const QVector<int>& labels) const
{
    int n = data.size();
    if (n <= 1 || m_numClusters < 2) return 0.0;

    double total = 0.0;
    for (int i = 0; i < n; ++i) {
        double a = 0.0; // Mean intra-cluster distance
        int aCount = 0;
        QVector<double> interDist(m_numClusters, 0.0);
        QVector<int> interCount(m_numClusters, 0);

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double d = qSqrt(distSq(data[i], data[j]));
            if (labels[j] == labels[i]) { a += d; ++aCount; }
            else { interDist[labels[j]] += d; ++interCount[labels[j]]; }
        }
        a = aCount > 0 ? a / aCount : 0.0;

        double b = 1e30;
        for (int c = 0; c < m_numClusters; ++c) {
            if (c == labels[i] || interCount[c] == 0) continue;
            b = qMin(b, interDist[c] / interCount[c]);
        }
        if (b > 1e29) b = 0.0;

        double denom = qMax(a, b);
        total += (denom > 0.0) ? (b - a) / denom : 0.0;
    }
    return total / n;
}

/* ---- Predict ---- */

int KMeans16::predict(const QVector<double>& sample) const
{
    if (m_centers.isEmpty()) return 0;
    double bestD = 1e30;
    int bestC = 0;
    for (int c = 0; c < m_centers.size(); ++c) {
        double d = distSq(sample, m_centers[c]);
        if (d < bestD) { bestD = d; bestC = c; }
    }
    return bestC;
}

/* ---- Main fit ---- */

QVector<int> KMeans16::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return {};
    m_dims = data[0].size();

    switch (m_mode) {
    case MiniBatch:    m_labels = miniBatchFit(data); break;
    case BisectingK:   m_labels = bisectingFit(data); break;
    default:           m_labels = lloydsIterate(data); break;
    }

    double inertia = computeInertia(data);
    double sil = silhouetteScore(data, m_labels);

    m_stats.totalRuns++;
    m_stats.numClusters = m_numClusters;
    m_stats.numSamples = m_n;
    m_stats.inertia = inertia;
    m_stats.silhouette = sil;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_numClusters, inertia, sil);
    return m_labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> KMeans16::centers() const { return m_centers; }

void KMeans16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
