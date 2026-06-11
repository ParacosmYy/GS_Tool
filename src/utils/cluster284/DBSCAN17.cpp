/**
 * @file DBSCAN17.cpp
 * @brief DBSCAN17 实现
 *
 * 实现DBSCAN密度聚类：边界点重分配与变epsilon密度自适应边界精化。
 */

#include "utils/cluster284/DBSCAN17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DBSCAN17::DBSCAN17(QObject *parent)
    : QObject(parent) {}

DBSCAN17::~DBSCAN17() = default;

/* ---- Configuration ---- */

void DBSCAN17::setEpsilon(double eps) { m_eps = qBound(0.001, eps, 1e6); }
void DBSCAN17::setMinPoints(int minPts) { m_minPts = qBound(2, minPts, 1000); }
void DBSCAN17::setEpsilonRatio(double ratio) { m_epsRatio = qBound(1.0, ratio, 10.0); }
void DBSCAN17::setMaxBorderIter(int iters) { m_maxBorderIter = qBound(1, iters, 20); }

/* ---- Euclidean distance ---- */

double DBSCAN17::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size() && i < b.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Region query ---- */

QVector<int> DBSCAN17::regionQuery(int idx, double eps) const
{
    QVector<int> neighbors;
    for (int i = 0; i < m_data.size(); ++i) {
        if (i == idx) continue;
        if (distance(m_data[idx], m_data[i]) <= eps)
            neighbors.append(i);
    }
    return neighbors;
}

/* ---- Expand cluster from core point ---- */

void DBSCAN17::expandCluster(int idx, int clusterId, double eps)
{
    QVector<int> seeds = regionQuery(idx, eps);
    if (seeds.size() < m_minPts - 1) return; // idx is not core

    m_isCore[idx] = true;
    m_labels[idx] = clusterId;

    // Mark direct neighbors as border initially
    for (int nIdx : seeds)
        if (m_labels[nIdx] == -1 || m_labels[nIdx] == 0)
            m_labels[nIdx] = clusterId;

    // BFS expansion
    int i = 0;
    while (i < seeds.size()) {
        int cur = seeds[i];
        if (!m_visited[cur]) {
            m_visited[cur] = true;
            double curEps = variableEpsilon(cur);
            QVector<int> curNeighbors = regionQuery(cur, curEps);
            if (curNeighbors.size() >= m_minPts - 1) {
                m_isCore[cur] = true;
                for (int nIdx : curNeighbors) {
                    if (m_labels[nIdx] <= 0) {
                        m_labels[nIdx] = clusterId;
                        seeds.append(nIdx);
                    }
                }
            }
        }
        i++;
    }
}

/* ---- Compute local density (k-distance) ---- */

double DBSCAN17::localDensity(int idx) const
{
    QVector<double> dists;
    dists.reserve(m_data.size() - 1);
    for (int i = 0; i < m_data.size(); ++i) {
        if (i == idx) continue;
        dists.append(distance(m_data[idx], m_data[i]));
    }
    std::sort(dists.begin(), dists.end());
    int k = qMin(m_minPts, dists.size());
    return (k > 0) ? dists[k - 1] : m_eps;
}

/* ---- Variable epsilon based on local density ---- */

double DBSCAN17::variableEpsilon(int idx) const
{
    double localEps = localDensity(idx);
    return (m_eps + localEps) * 0.5;
}

/* ---- Border point reassignment ---- */

void DBSCAN17::reassignBorders()
{
    for (int iter = 0; iter < m_maxBorderIter; ++iter) {
        bool changed = false;
        for (int i = 0; i < m_data.size(); ++i) {
            if (m_isCore[i] || m_labels[i] == -1) continue; // Skip core and noise

            // Count neighbors per cluster within expanded epsilon
            double borderEps = m_eps * m_epsRatio;
            QVector<int> neighbors = regionQuery(i, borderEps);

            QMap<int, int> clusterCounts;
            for (int nIdx : neighbors)
                if (m_labels[nIdx] > 0)
                    clusterCounts[m_labels[nIdx]]++;

            // Find dominant cluster
            int bestCluster = m_labels[i];
            int bestCount = 0;
            for (auto it = clusterCounts.constBegin(); it != clusterCounts.constEnd(); ++it) {
                if (it.value() > bestCount) {
                    bestCount = it.value();
                    bestCluster = it.key();
                }
            }
            if (bestCluster != m_labels[i]) {
                m_labels[i] = bestCluster;
                changed = true;
            }
        }
        if (!changed) break;
    }
}

/* ---- Main fit ---- */

DBSCAN17::ClusterResult DBSCAN17::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n == 0) return result;

    m_data = data;
    m_labels.fill(0, n);       // 0 = unvisited
    m_visited.fill(false, n);
    m_isCore.fill(false, n);

    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (m_visited[i]) continue;
        m_visited[i] = true;

        double eps = variableEpsilon(i);
        QVector<int> neighbors = regionQuery(i, eps);

        if (neighbors.size() < m_minPts - 1) {
            m_labels[i] = -1;  // Noise
            continue;
        }

        clusterId++;
        expandCluster(i, clusterId, eps);
    }

    // Border reassignment phase
    reassignBorders();

    // Build result
    result.labels = m_labels;
    result.numClusters = clusterId;
    result.numNoise = 0;

    result.clusters.resize(clusterId + 1);
    for (int i = 0; i < n; ++i) {
        if (m_labels[i] == -1) {
            result.numNoise++;
        } else if (m_labels[i] > 0 && m_labels[i] <= clusterId) {
            result.clusters[m_labels[i]].append(i);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = clusterId;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, clusterId, result.numNoise, elapsed);
    return result;
}

/* ---- Predict ---- */

QVector<int> DBSCAN17::predict(const QVector<QVector<double>>& samples,
                                const QVector<QVector<double>>& trainData) const
{
    QVector<int> result(samples.size(), -1);
    for (int i = 0; i < samples.size(); ++i) {
        double bestDist = 1e30;
        int bestIdx = -1;
        for (int j = 0; j < trainData.size(); ++j) {
            double d = distance(samples[i], trainData[j]);
            if (d < bestDist) { bestDist = d; bestIdx = j; }
        }
        if (bestIdx >= 0 && bestIdx < m_labels.size())
            result[i] = m_labels[bestIdx];
    }
    return result;
}

/* ---- Reset ---- */

void DBSCAN17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_labels.clear();
    m_visited.clear();
    m_isCore.clear();
    m_data.clear();
}
