/**
 * @file DBSCAN16.cpp
 * @brief DBSCAN16 实现
 *
 * 实现DBSCAN密度聚类：空间索引网格加速与核心/边界点分类。
 */

#include "utils/cluster270/DBSCAN16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DBSCAN16::DBSCAN16(QObject *parent)
    : QObject(parent) {}

DBSCAN16::~DBSCAN16() = default;

/* ---- Configuration ---- */

void DBSCAN16::setEpsilon(double eps)
{
    m_epsilon = qBound(0.001, eps, 1e6);
}

void DBSCAN16::setMinPoints(int minPts)
{
    m_minPts = qBound(2, minPts, 1000);
}

/* ---- Grid key computation for 2D spatial hashing ---- */

qint64 DBSCAN16::gridKey(double x, double y, double cellSize) const
{
    int gx = qFloor(x / cellSize);
    int gy = qFloor(y / cellSize);
    // Combine into a single 64-bit key using Cantor pairing variant
    qint64 ax = static_cast<qint64>(gx) + 1000000;
    qint64 ay = static_cast<qint64>(gy) + 1000000;
    return ax * 2000003LL + ay;
}

/* ---- Open-addressing hash map for grid cells ---- */

int DBSCAN16::gridFind(qint64 key) const
{
    if (m_gridCapacity == 0) return -1;
    int idx = static_cast<int>((key * 2654435761ULL) % m_gridCapacity);
    for (int probe = 0; probe < m_gridCapacity; ++probe) {
        int slot = (idx + probe) % m_gridCapacity;
        if (m_gridMap[slot].first == -1) return -1;
        if (m_gridMap[slot].first == key) return m_gridMap[slot].second;
    }
    return -1;
}

int DBSCAN16::gridInsert(qint64 key)
{
    int idx = static_cast<int>((key * 2654435761ULL) % m_gridCapacity);
    for (int probe = 0; probe < m_gridCapacity; ++probe) {
        int slot = (idx + probe) % m_gridCapacity;
        if (m_gridMap[slot].first == -1 || m_gridMap[slot].first == key) {
            if (m_gridMap[slot].first == key) return m_gridMap[slot].second;
            m_gridMap[slot] = {key, m_gridCells.size()};
            m_gridCells.append(GridCell{});
            return m_gridMap[slot].second;
        }
    }
    return -1;
}

/* ---- Build spatial index grid ---- */

void DBSCAN16::buildGrid(const QVector<QVector<double>>& data, double cellSize)
{
    int n = data.size();
    m_gridCapacity = qMax(16, n * 2);
    m_gridMap.assign(m_gridCapacity, {-1, -1});
    m_gridCells.clear();

    for (int i = 0; i < n; ++i) {
        qint64 key = gridKey(data[i][0], data[i][1], cellSize);
        int cellIdx = gridInsert(key);
        if (cellIdx >= 0)
            m_gridCells[cellIdx].pointIndices.append(i);
    }
}

/* ---- Region query using spatial grid ---- */

QVector<int> DBSCAN16::regionQuery(const QVector<QVector<double>>& data,
                                    int idx, double cellSize) const
{
    QVector<int> neighbors;
    double x0 = data[idx][0];
    double y0 = data[idx][1];
    double eps2 = m_epsilon * m_epsilon;

    // Check the 3x3 neighborhood of grid cells
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            qint64 key = gridKey(x0 + dx * cellSize, y0 + dy * cellSize, cellSize);
            int cellIdx = gridFind(key);
            if (cellIdx < 0) continue;

            for (int j : m_gridCells[cellIdx].pointIndices) {
                if (j == idx) continue;
                double ddx = data[j][0] - x0;
                double ddy = data[j][1] - y0;
                if (ddx * ddx + ddy * ddy <= eps2)
                    neighbors.append(j);
            }
        }
    }
    return neighbors;
}

/* ---- Expand cluster via BFS with seed set ---- */

void DBSCAN16::expandCluster(const QVector<QVector<double>>& data, int startIdx,
                              int clusterId, double cellSize,
                              const QVector<QVector<int>>& neighborCache)
{
    QVector<int> queue;
    queue.append(startIdx);
    m_labels[startIdx] = clusterId;

    int head = 0;
    while (head < queue.size()) {
        int cur = queue[head++];
        const QVector<int>& neighbors = neighborCache[cur];

        if (neighbors.size() + 1 >= m_minPts) {
            m_pointTypes[cur] = Core;
            for (int nb : neighbors) {
                if (m_labels[nb] < 0) {
                    m_labels[nb] = clusterId;
                    m_pointTypes[nb] = Border;
                    queue.append(nb);
                }
            }
        } else if (m_pointTypes[cur] != Core) {
            m_pointTypes[cur] = Border;
        }
    }
}

/* ---- Main fit ---- */

QVector<int> DBSCAN16::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    // Labels: -1 = unvisited, -2 = noise, >= 0 = cluster id
    m_labels.fill(-1, n);
    m_pointTypes.fill(Noise, n);

    // Build spatial index grid with cell size = epsilon
    buildGrid(data, m_epsilon);

    // Pre-compute neighbors for all points using grid
    QVector<QVector<int>> neighborCache(n);
    for (int i = 0; i < n; ++i)
        neighborCache[i] = regionQuery(data, i, m_epsilon);

    int clusterId = 0;
    for (int i = 0; i < n; ++i) {
        if (m_labels[i] >= 0) continue;

        const QVector<int>& neighbors = neighborCache[i];
        if (neighbors.size() + 1 < m_minPts) {
            // Mark as noise
            m_labels[i] = -2;
            m_pointTypes[i] = Noise;
            continue;
        }

        // Core point found, expand cluster
        m_pointTypes[i] = Core;
        expandCluster(data, i, clusterId, m_epsilon, neighborCache);
        clusterId++;
    }

    m_numClusters = clusterId;

    // Convert labels: -2 (noise) stays as -1 in output
    QVector<int> result = m_labels;
    for (int i = 0; i < n; ++i) {
        if (result[i] == -2) result[i] = -1;
    }

    // Collect statistics
    int numCore = 0, numBorder = 0, numNoise = 0;
    for (int i = 0; i < n; ++i) {
        if (m_pointTypes[i] == Core) numCore++;
        else if (m_pointTypes[i] == Border) numBorder++;
        else numNoise++;
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_numClusters;
    m_stats.numCore = numCore;
    m_stats.numBorder = numBorder;
    m_stats.numNoise = numNoise;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringDone(m_numClusters, numCore, numBorder, numNoise, elapsed);

    return result;
}

/* ---- Accessors ---- */

QVector<int> DBSCAN16::pointTypes() const { return m_pointTypes; }
int DBSCAN16::numClusters() const { return m_numClusters; }
QVector<int> DBSCAN16::labels() const { return m_labels; }

/* ---- Reset ---- */

void DBSCAN16::resetStatistics()
{
    m_labels.clear();
    m_pointTypes.clear();
    m_numClusters = 0;
    m_gridCells.clear();
    m_gridMap.clear();
    m_gridCapacity = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
