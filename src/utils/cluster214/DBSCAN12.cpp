/**
 * @file DBSCAN12.cpp
 * @brief DBSCAN12 实现
 *
 * 实现DBSCAN密度聚类：k-distance肘部法则自适应epsilon、边界点置信度评分、BFS扩展。
 */

#include "utils/cluster214/DBSCAN12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DBSCAN12::DBSCAN12(QObject *parent) : QObject(parent) {}
DBSCAN12::~DBSCAN12() = default;

/* ---- Configuration ---- */

void DBSCAN12::setParameters(int minPts, double epsilon)
{
    m_minPts = qMax(2, minPts);
    m_epsilon = qMax(0.0, epsilon);
}

/* ---- Euclidean distance ---- */

double DBSCAN12::distance(const QVector<double>& a,
                           const QVector<double>& b) const
{
    double d2 = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i)
        d2 += qPow(a[i] - b[i], 2);
    return qSqrt(d2);
}

/* ---- K-distance graph for elbow detection ---- */

QVector<double> DBSCAN12::kDistanceGraph(int k) const
{
    QVector<double> kd(m_n);
    for (int i = 0; i < m_n; ++i) {
        QVector<double> dists;
        dists.reserve(m_n - 1);
        for (int j = 0; j < m_n; ++j) {
            if (i != j)
                dists.append(distance(m_data[i], m_data[j]));
        }
        std::sort(dists.begin(), dists.end());
        int idx = qMin(k - 1, dists.size() - 1);
        kd[i] = (idx >= 0) ? dists[idx] : 0.0;
    }
    std::sort(kd.begin(), kd.end());
    return kd;
}

/* ---- Auto-select epsilon via elbow ---- */

double DBSCAN12::autoEpsilon() const
{
    QVector<double> kd = kDistanceGraph(m_minPts);
    if (kd.size() < 3) return 1.0;

    // Find maximum curvature point
    double maxCurv = 0.0;
    double bestEps = kd[kd.size() / 2];
    int n = kd.size();

    for (int i = 1; i < n - 1; ++i) {
        // Second derivative approximation for curvature
        double dx = 1.0 / n;
        double d1 = (kd[i + 1] - kd[i - 1]) / (2.0 * dx);
        double d2 = (kd[i + 1] - 2.0 * kd[i] + kd[i - 1]) / (dx * dx);
        double denom = qPow(1.0 + d1 * d1, 1.5);
        double curv = qAbs(d2) / qMax(denom, 1e-12);
        if (curv > maxCurv) {
            maxCurv = curv;
            bestEps = kd[i];
        }
    }
    return qMax(1e-6, bestEps);
}

/* ---- Range query: find neighbors within epsilon ---- */

QVector<int> DBSCAN12::rangeQuery(int pointIdx) const
{
    QVector<int> neighbors;
    for (int j = 0; j < m_n; ++j) {
        if (distance(m_data[pointIdx], m_data[j]) <= m_epsilon)
            neighbors.append(j);
    }
    return neighbors;
}

/* ---- Boundary point confidence ---- */

double DBSCAN12::computeConfidence(int pointIdx,
                                    const QVector<int>& neighbors) const
{
    // Confidence = ratio of neighbors that are core points
    int coreNeighbors = 0;
    for (int nb : neighbors) {
        // A core point has >= minPts neighbors
        QVector<int> nbNeighbors = rangeQuery(nb);
        if (nbNeighbors.size() >= m_minPts)
            coreNeighbors++;
    }
    return (neighbors.size() > 1)
        ? static_cast<double>(coreNeighbors) / (neighbors.size() - 1)
        : 0.0;
}

/* ---- Expand cluster via BFS ---- */

void DBSCAN12::expandCluster(int pointIdx, int clusterId,
                              const QVector<QVector<int>>& neighborLists)
{
    QVector<int> queue;
    queue.append(pointIdx);
    m_labels[pointIdx] = clusterId;

    int head = 0;
    while (head < queue.size()) {
        int cur = queue[head++];
        const QVector<int>& neighbors = neighborLists[cur];

        if (neighbors.size() >= m_minPts) {
            m_types[cur] = Core;
            for (int nb : neighbors) {
                if (m_labels[nb] == -1) {  // Unvisited
                    m_labels[nb] = clusterId;
                    queue.append(nb);
                    if (neighborLists[nb].size() >= m_minPts)
                        m_types[nb] = Core;
                    else
                        m_types[nb] = Boundary;
                } else if (m_labels[nb] == -2) {  // Previously noise
                    m_labels[nb] = clusterId;
                    m_types[nb] = Boundary;
                }
            }
        }
    }
}

/* ---- Fit ---- */

void DBSCAN12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    m_n = data.size();
    if (m_n == 0) return;
    m_dim = data[0].size();

    // Auto-detect epsilon if not set
    if (m_epsilon <= 0.0)
        m_epsilon = autoEpsilon();

    m_labels.resize(m_n, -1);      // -1 = unvisited
    m_types.resize(m_n, Noise);
    m_confidence.resize(m_n, 0.0);

    // Precompute neighbor lists
    QVector<QVector<int>> neighborLists(m_n);
    for (int i = 0; i < m_n; ++i)
        neighborLists[i] = rangeQuery(i);

    int clusterId = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_labels[i] != -1) continue;   // Already processed

        if (neighborLists[i].size() < m_minPts) {
            m_labels[i] = -2;   // Noise
            m_types[i] = Noise;
            continue;
        }

        // Expand cluster from this core point
        expandCluster(i, clusterId, neighborLists);
        clusterId++;
    }

    // Compute boundary confidence scores
    for (int i = 0; i < m_n; ++i) {
        if (m_types[i] == Boundary)
            m_confidence[i] = computeConfidence(i, neighborLists[i]);
    }

    // Update stats
    m_stats.numSamples = m_n;
    m_stats.numClusters = clusterId;
    m_stats.dimensions = m_dim;
    m_stats.epsilon = m_epsilon;
    m_stats.coreCount = 0;
    m_stats.boundaryCount = 0;
    m_stats.noiseCount = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_types[i] == Core) m_stats.coreCount++;
        else if (m_types[i] == Boundary) m_stats.boundaryCount++;
        else m_stats.noiseCount++;
    }
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(clusterId, m_stats.coreCount,
                              m_stats.boundaryCount, m_stats.noiseCount,
                              timer.elapsed());
}

/* ---- Accessors ---- */

QVector<int> DBSCAN12::labels() const { return m_labels; }
QVector<DBSCAN12::PointType> DBSCAN12::pointTypes() const { return m_types; }
QVector<double> DBSCAN12::boundaryConfidence() const { return m_confidence; }

/* ---- Reset ---- */

void DBSCAN12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_data.clear();
    m_labels.clear();
    m_types.clear();
    m_confidence.clear();
    m_n = 0;
    m_dim = 0;
}
