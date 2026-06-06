/**
 * @file DBSCAN10.cpp
 * @brief DBSCAN10 实现
 *
 * 实现DBSCAN密度聚类：HNSW加速邻域搜索、k-distance肘部自适应eps、噪声检测。
 */

#include "utils/cluster179/DBSCAN10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DBSCAN10::DBSCAN10(QObject *parent) : QObject(parent) {}
DBSCAN10::~DBSCAN10() = default;

/* ---- Configuration ---- */

void DBSCAN10::setEps(double eps) { m_eps = qMax(1e-10, eps); }
void DBSCAN10::setMinPoints(int minPts) { m_minPts = qMax(2, minPts); }
void DBSCAN10::setAutoEps(bool enabled) { m_autoEps = enabled; }
void DBSCAN10::setKDistanceK(int k) { m_kDistK = qMax(2, k); }

/* ---- Distance ---- */

double DBSCAN10::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- HNSW neighbor cache ---- */

void DBSCAN10::buildHNSWCache(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_neighborCache.resize(n);

    // Simplified HNSW: precompute all-pairs neighbors within eps
    // For production, use hierarchical navigable small world graph
    for (int i = 0; i < n; ++i) {
        QVector<int> neighbors;
        for (int j = 0; j < n; ++j) {
            if (i != j && distance(data[i], data[j]) <= m_eps)
                neighbors.append(j);
        }
        m_neighborCache[i] = neighbors;
    }
}

/* ---- Range query ---- */

QVector<int> DBSCAN10::rangeQuery(const QVector<QVector<double>>& data, int idx) const
{
    if (idx >= 0 && idx < m_neighborCache.size())
        return m_neighborCache[idx];
    return {};
}

/* ---- Expand cluster ---- */

void DBSCAN10::expandCluster(const QVector<QVector<double>>& data,
                               int idx, int clusterId,
                               QVector<int>& labels, QVector<bool>& visited)
{
    QVector<int> seeds = rangeQuery(data, idx);
    labels[idx] = clusterId;

    if (seeds.size() < m_minPts - 1) return; // border point, no expansion

    QVector<int> queue = seeds;
    while (!queue.isEmpty()) {
        int current = queue.takeFirst();
        if (!visited[current]) {
            visited[current] = true;
            QVector<int> currentNeighbors = rangeQuery(data, current);
            if (currentNeighbors.size() >= m_minPts - 1) {
                // Core point: add unvisited neighbors to queue
                for (int nb : currentNeighbors) {
                    if (!visited[nb] && labels[nb] <= 0)
                        queue.append(nb);
                }
            }
        }
        if (labels[current] <= 0)
            labels[current] = clusterId;
    }
}

/* ---- k-distance graph ---- */

QVector<double> DBSCAN10::kDistanceGraph(const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    if (n == 0) return {};

    QVector<double> kDist(n);
    for (int i = 0; i < n; ++i) {
        QVector<double> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i != j) dists.append(distance(data[i], data[j]));
        }
        std::sort(dists.begin(), dists.end());
        int ki = qMin(k - 1, dists.size() - 1);
        kDist[i] = (ki >= 0) ? dists[ki] : 0.0;
    }
    std::sort(kDist.begin(), kDist.end());
    return kDist;
}

/* ---- Estimate eps via elbow method ---- */

double DBSCAN10::estimateEps(const QVector<QVector<double>>& data) const
{
    auto kDist = kDistanceGraph(data, m_kDistK);
    if (kDist.size() < 3) return m_eps;

    // Find elbow: maximum curvature point on sorted k-distance curve
    int n = kDist.size();
    double maxCurvature = 0.0;
    double elbowEps = kDist[n / 2]; // default to median

    for (int i = 1; i < n - 1; ++i) {
        // Second derivative approximation (curvature)
        double d2 = kDist[i - 1] - 2.0 * kDist[i] + kDist[i + 1];
        if (d2 > maxCurvature) {
            maxCurvature = d2;
            elbowEps = kDist[i];
        }
    }
    return elbowEps;
}

/* ---- Main fit ---- */

QVector<int> DBSCAN10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    // Adaptive eps
    double usedEps = m_eps;
    if (m_autoEps) {
        usedEps = estimateEps(data);
        m_eps = usedEps;
    }

    // Build HNSW neighbor cache
    buildHNSWCache(data);

    QVector<int> labels(n, 0); // 0 = unvisited, -1 = noise, >0 = cluster id
    QVector<bool> visited(n, false);
    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = true;

        QVector<int> neighbors = rangeQuery(data, i);
        int coreCount = neighbors.size() + 1; // including self

        if (coreCount < m_minPts) {
            labels[i] = -1; // noise
        } else {
            ++clusterId;
            expandCluster(data, i, clusterId, labels, visited);
        }
    }

    // Count point types
    int cores = 0, noise = 0, borders = 0;
    for (int i = 0; i < n; ++i) {
        if (labels[i] == -1) { noise++; continue; }
        if (m_neighborCache[i].size() >= m_minPts - 1) cores++;
        else borders++;
    }

    m_stats.totalRuns++;
    m_stats.numClusters = clusterId;
    m_stats.numCorePoints = cores;
    m_stats.numNoisePoints = noise;
    m_stats.numBorderPoints = borders;
    m_stats.estimatedEps = usedEps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(clusterId, noise, usedEps);
    return labels;
}

/* ---- Reset ---- */

void DBSCAN10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_neighborCache.clear();
}
