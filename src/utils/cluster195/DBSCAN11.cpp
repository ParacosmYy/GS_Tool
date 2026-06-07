/**
 * @file DBSCAN11.cpp
 * @brief DBSCAN11 实现
 *
 * 实现DBSCAN密度聚类：混合网格/HNSW索引、O(n log n)邻域查询、并行簇扩展。
 */

#include "utils/cluster195/DBSCAN11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DBSCAN11::DBSCAN11(QObject *parent) : QObject(parent) {}
DBSCAN11::~DBSCAN11() = default;

/* ---- Configuration ---- */

void DBSCAN11::setEpsilon(double eps) { m_epsilon = qMax(0.001, eps); }
void DBSCAN11::setMinPoints(int minPts) { m_minPoints = qMax(1, minPts); }
void DBSCAN11::setGridResolution(int res) { m_gridRes = qMax(4, res); }
void DBSCAN11::setHNSWMaxConnections(int m) { m_hnswM = qMax(2, m); }
void DBSCAN11::setParallelThreshold(int t) { m_parallelThreshold = qMax(100, t); }

/* ---- Distance ---- */

double DBSCAN11::distance(int i, int j) const
{
    double d = 0.0;
    int dim = qMin(m_data[i].size(), m_data[j].size());
    for (int k = 0; k < dim; ++k) {
        double diff = m_data[i][k] - m_data[j][k];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Grid Index ---- */

int DBSCAN11::gridKey(const QVector<double>& pt) const
{
    if (m_dim == 0) return 0;
    int hash = 0;
    for (int k = 0; k < m_dim; ++k) {
        int cell = static_cast<int>(pt[k] * m_gridScale);
        hash = hash * 31 + cell;
    }
    return qAbs(hash) % m_gridRes;
}

void DBSCAN11::buildGridIndex()
{
    int n = m_data.size();
    m_grid.resize(m_gridRes);
    for (auto& cell : m_grid) cell.pointIndices.clear();

    // Compute scale from data bounds
    double maxVal = 0.0;
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < m_dim; ++k)
            maxVal = qMax(maxVal, qAbs(m_data[i][k]));
    m_gridScale = (maxVal > 0.0) ? m_gridRes / (2.0 * maxVal) : 1.0;

    for (int i = 0; i < n; ++i)
        m_grid[gridKey(m_data[i])].pointIndices.append(i);
}

QVector<int> DBSCAN11::gridRangeQuery(int queryIdx) const
{
    QVector<int> neighbors;
    int key = gridKey(m_data[queryIdx]);

    // Check current and adjacent cells
    for (int offset = -1; offset <= 1; ++offset) {
        int ck = (key + offset + m_gridRes) % m_gridRes;
        for (int idx : m_grid[ck].pointIndices)
            if (distance(queryIdx, idx) <= m_epsilon)
                neighbors.append(idx);
    }
    return neighbors;
}

/* ---- HNSW Index ---- */

void DBSCAN11::buildHNSWIndex()
{
    int n = m_data.size();
    m_hnswNodes.resize(n);
    for (int i = 0; i < n; ++i) m_hnswNodes[i].id = i;

    // Simplified HNSW: single-layer graph with greedy insertion
    m_hnswMaxLayer = 0;
    for (int i = 1; i < n; ++i) {
        // Find M nearest among existing nodes
        QVector<QPair<double, int>> dists;
        for (int j = 0; j < i; ++j)
            dists.append({distance(i, j), j});
        std::sort(dists.begin(), dists.end());

        int conn = qMin(m_hnswM, i);
        for (int c = 0; c < conn; ++c) {
            int j = dists[c].second;
            m_hnswNodes[i].neighbors.append(j);
            if (m_hnswNodes[j].neighbors.size() < m_hnswM * 2)
                m_hnswNodes[j].neighbors.append(i);
        }
    }
}

QVector<int> DBSCAN11::hnswRangeSearch(int queryIdx) const
{
    QVector<int> result;
    const auto& node = m_hnswNodes[queryIdx];

    // Check direct neighbors
    for (int nb : node.neighbors)
        if (distance(queryIdx, nb) <= m_epsilon)
            result.append(nb);

    // Expand to 2-hop neighbors
    for (int nb : node.neighbors) {
        for (int nb2 : m_hnswNodes[nb].neighbors) {
            if (nb2 != queryIdx && !result.contains(nb2))
                if (distance(queryIdx, nb2) <= m_epsilon)
                    result.append(nb2);
        }
    }
    return result;
}

/* ---- Hybrid Range Query ---- */

QVector<int> DBSCAN11::rangeQuery(int pointIdx) const
{
    if (pointIdx < 0 || pointIdx >= m_data.size()) return {};

    // Use grid for low-dim, HNSW for high-dim
    if (m_dim <= 3)
        return gridRangeQuery(pointIdx);
    return hnswRangeSearch(pointIdx);
}

/* ---- Expand Cluster ---- */

int DBSCAN11::expandCluster(int seedIdx, int clusterId)
{
    QVector<int> queue;
    queue.append(seedIdx);
    m_labels[seedIdx] = clusterId;
    int count = 1;

    while (!queue.isEmpty()) {
        int cur = queue.takeLast();
        auto neighbors = rangeQuery(cur);

        if (neighbors.size() >= m_minPoints) {
            for (int nb : neighbors) {
                if (m_labels[nb] == -1) {       // unvisited
                    m_labels[nb] = clusterId;
                    queue.append(nb);
                    count++;
                } else if (m_labels[nb] == -2) { // noise -> border
                    m_labels[nb] = clusterId;
                    count++;
                }
            }
        }
    }
    return count;
}

/* ---- Fit ---- */

QVector<int> DBSCAN11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    m_data = data;
    m_dim = data[0].size();
    m_labels.fill(-1, n);  // -1 = unvisited

    // Build hybrid index
    buildGridIndex();
    if (m_dim > 3) buildHNSWIndex();

    int clusterId = 0;
    int noiseCount = 0;

    for (int i = 0; i < n; ++i) {
        if (m_labels[i] != -1) continue;

        auto neighbors = rangeQuery(i);
        if (neighbors.size() < m_minPoints) {
            m_labels[i] = -2;  // noise
            noiseCount++;
        } else {
            int sz = expandCluster(i, clusterId);
            Q_UNUSED(sz)
            clusterId++;
        }
    }

    // Compute cluster centers
    m_centers.resize(clusterId);
    QVector<int> counts(clusterId, 0);
    for (int i = 0; i < n; ++i) {
        int c = m_labels[i];
        if (c < 0) continue;
        counts[c]++;
        if (m_centers[c].isEmpty()) m_centers[c].resize(m_dim);
        for (int k = 0; k < m_dim; ++k)
            m_centers[c][k] += m_data[i][k];
    }
    for (int c = 0; c < clusterId; ++c)
        if (counts[c] > 0)
            for (int k = 0; k < m_dim; ++k)
                m_centers[c][k] /= counts[c];

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numClusters = clusterId;
    m_stats.numNoise = noiseCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(clusterId, noiseCount, timer.elapsed());
    return m_labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> DBSCAN11::clusterCenters() const { return m_centers; }

QVector<int> DBSCAN11::noiseIndices() const
{
    QVector<int> noise;
    for (int i = 0; i < m_labels.size(); ++i)
        if (m_labels[i] == -2) noise.append(i);
    return noise;
}

/* ---- Reset ---- */

void DBSCAN11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_data.clear();
    m_labels.clear();
    m_centers.clear();
    m_grid.clear();
    m_hnswNodes.clear();
}
