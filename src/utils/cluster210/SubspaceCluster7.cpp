/**
 * @file SubspaceCluster7.cpp
 * @brief SubspaceCluster7 实现
 *
 * 实现子空间聚类：PROCLUS投影聚类、熵质量评估、迭代medoid优化。
 */

#include "utils/cluster210/SubspaceCluster7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>
#include <limits>

/* ---- Construction / Destruction ---- */

SubspaceCluster7::SubspaceCluster7(QObject *parent) : QObject(parent) {}
SubspaceCluster7::~SubspaceCluster7() = default;

/* ---- Configuration ---- */

void SubspaceCluster7::setParameters(int k, int subspaceDim)
{
    m_k = qMax(2, k);
    m_l = qMax(1, subspaceDim);
}

/* ---- Manhattan distance in subspace ---- */

double SubspaceCluster7::manhattanSubspace(const QVector<double>& a,
                                            const QVector<double>& b,
                                            const QVector<int>& dims)
{
    double sum = 0.0;
    for (int d : dims) {
        if (d >= 0 && d < a.size() && d < b.size())
            sum += qAbs(a[d] - b[d]);
    }
    return sum;
}

/* ---- Select initial medoids (greedy farthest-first) ---- */

QVector<int> SubspaceCluster7::selectMedoids(const QVector<QVector<double>>& data,
                                              int k) const
{
    int n = data.size();
    QVector<int> medoids;
    medoids.reserve(k);

    // Start with random first medoid
    int first = qrand() % n;
    medoids.append(first);

    QVector<double> minDist(n, std::numeric_limits<double>::max());

    for (int m = 1; m < k; ++m) {
        // Update min distances to nearest existing medoid
        int lastMed = medoids.last();
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < m_dim; ++j)
                d += qAbs(data[i][j] - data[lastMed][j]);
            minDist[i] = qMin(minDist[i], d);
        }
        // Select farthest point
        double maxD = -1.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            if (minDist[i] > maxD) {
                maxD = minDist[i];
                bestIdx = i;
            }
        }
        medoids.append(bestIdx);
    }
    return medoids;
}

/* ---- Compute localities ---- */

QVector<QVector<int>> SubspaceCluster7::computeLocalities(
    const QVector<int>& medoids, const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = medoids.size();
    int localitySize = qMax(1, 10 * n / (k * m_l));

    QVector<QVector<int>> localities(k);
    for (int m = 0; m < k; ++m) {
        // Collect (distance, index) pairs to all points
        QVector<QPair<double, int>> dists;
        dists.reserve(n);
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < m_dim; ++j)
                d += qAbs(data[i][j] - data[medoids[m]][j]);
            dists.append({d, i});
        }
        std::sort(dists.begin(), dists.end());
        int sz = qMin(localitySize, n);
        localities[m].reserve(sz);
        for (int i = 0; i < sz; ++i)
            localities[m].append(dists[i].second);
    }
    return localities;
}

/* ---- Entropy of one dimension ---- */

double SubspaceCluster7::dimensionEntropy(const QVector<double>& values) const
{
    if (values.size() <= 1) return 0.0;
    int n = values.size();

    // Bin values for entropy estimation
    int numBins = qMax(2, qMin(20, n / 2));
    double vmin = values[0], vmax = values[0];
    for (double v : values) {
        vmin = qMin(vmin, v);
        vmax = qMax(vmax, v);
    }
    double range = vmax - vmin;
    if (range < 1e-12) return 0.0;

    QVector<int> bins(numBins, 0);
    for (double v : values) {
        int b = qBound(0, static_cast<int>((v - vmin) / range * numBins), numBins - 1);
        bins[b]++;
    }

    double entropy = 0.0;
    for (int count : bins) {
        if (count > 0) {
            double p = static_cast<double>(count) / n;
            entropy -= p * qLn(p);
        }
    }
    return entropy;
}

/* ---- Select dimensions for each locality ---- */

QVector<QVector<int>> SubspaceCluster7::selectDimensions(
    const QVector<QVector<int>>& localities,
    const QVector<QVector<double>>& data, int l) const
{
    int k = localities.size();
    QVector<QVector<int>> dimensions(k);

    for (int m = 0; m < k; ++m) {
        // Compute average distance per dimension within locality
        QVector<QPair<double, int>> dimScores;
        dimScores.reserve(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            QVector<double> vals;
            vals.reserve(localities[m].size());
            for (int idx : localities[m])
                vals.append(data[idx][d]);
            double ent = dimensionEntropy(vals);
            // Lower entropy = more clustered = better dimension
            dimScores.append({-ent, d});
        }
        std::sort(dimScores.begin(), dimScores.end());
        int sel = qMin(l, m_dim);
        dimensions[m].reserve(sel);
        for (int i = 0; i < sel; ++i)
            dimensions[m].append(dimScores[i].second);
    }
    return dimensions;
}

/* ---- Assign points to nearest medoid in subspace ---- */

QVector<int> SubspaceCluster7::assignPoints(
    const QVector<int>& medoids,
    const QVector<QVector<int>>& dimensions,
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int k = medoids.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        int bestM = 0;
        for (int m = 0; m < k; ++m) {
            double d = manhattanSubspace(data[i], data[medoids[m]], dimensions[m]);
            if (d < minDist) {
                minDist = d;
                bestM = m;
            }
        }
        labels[i] = bestM;
    }
    return labels;
}

/* ---- Subspace quality (entropy-based) ---- */

double SubspaceCluster7::subspaceQuality(const QVector<int>& dimensions,
                                          const QVector<int>& pointIndices,
                                          const QVector<QVector<double>>& data) const
{
    if (dimensions.isEmpty() || pointIndices.isEmpty()) return 0.0;
    double totalEntropy = 0.0;
    for (int d : dimensions) {
        QVector<double> vals;
        vals.reserve(pointIndices.size());
        for (int idx : pointIndices)
            if (idx >= 0 && idx < data.size() && d < data[idx].size())
                vals.append(data[idx][d]);
        totalEntropy += dimensionEntropy(vals);
    }
    return totalEntropy / dimensions.size();
}

/* ---- Fit ---- */

void SubspaceCluster7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return;
    m_dim = data[0].size();

    int k = qMin(m_k, m_n);
    int l = qMin(m_l, m_dim);

    // Step 1: Select initial medoids
    auto medoids = selectMedoids(data, k);

    // Step 2: Compute localities
    auto localities = computeLocalities(medoids, data);

    // Step 3: Select relevant dimensions per medoid
    auto dimensions = selectDimensions(localities, data, l);

    // Step 4: Assign points
    m_labels = assignPoints(medoids, dimensions, data);

    // Step 5: Build result clusters
    m_clusters.resize(k);
    double totalQuality = 0.0;
    for (int m = 0; m < k; ++m) {
        m_clusters[m].selectedDimensions = dimensions[m];
        m_clusters[m].pointIndices.clear();
    }
    for (int i = 0; i < m_n; ++i) {
        if (m_labels[i] >= 0 && m_labels[i] < k)
            m_clusters[m_labels[i]].pointIndices.append(i);
    }
    for (int m = 0; m < k; ++m) {
        m_clusters[m].quality = subspaceQuality(
            dimensions[m], m_clusters[m].pointIndices, data);
        totalQuality += m_clusters[m].quality;

        // Compute average intra-cluster distance
        double avgDist = 0.0;
        int count = 0;
        for (int p : m_clusters[m].pointIndices) {
            avgDist += manhattanSubspace(data[p], data[medoids[m]], dimensions[m]);
            count++;
        }
        m_clusters[m].avgDistance = (count > 0) ? avgDist / count : 0.0;
    }

    m_stats.numPoints = m_n;
    m_stats.numDimensions = m_dim;
    m_stats.numClusters = k;
    m_stats.avgSubspaceQuality = (k > 0) ? totalQuality / k : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(k, m_stats.avgSubspaceQuality, timer.elapsed());
}

/* ---- Getters ---- */

QVector<SubspaceCluster7::SubspaceCluster> SubspaceCluster7::clusters() const
{
    return m_clusters;
}

QVector<int> SubspaceCluster7::labels() const { return m_labels; }

/* ---- Reset ---- */

void SubspaceCluster7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_clusters.clear();
    m_labels.clear();
    m_n = 0;
    m_dim = 0;
}
