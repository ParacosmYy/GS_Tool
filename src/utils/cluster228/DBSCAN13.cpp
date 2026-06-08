/**
 * @file DBSCAN13.cpp
 * @brief DBSCAN13 实现
 *
 * 实现DBSCAN密度聚类：HNSW加速近邻搜索与局部可达密度自适应epsilon。
 */

#include "utils/cluster228/DBSCAN13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DBSCAN13::DBSCAN13(QObject *parent) : QObject(parent) {}
DBSCAN13::~DBSCAN13() = default;

/* ---- Configuration ---- */

void DBSCAN13::setParameters(double epsilon, int minPts, int hnswM)
{
    m_epsilon = qMax(0.001, epsilon);
    m_minPts = qMax(2, minPts);
    m_hnswM = qMax(4, hnswM);
}

/* ---- Euclidean distance ---- */

double DBSCAN13::distance(int i, int j) const
{
    double sum = 0.0;
    int d = qMin(m_data[i].size(), m_data[j].size());
    for (int k = 0; k < d; ++k) {
        double diff = m_data[i][k] - m_data[j][k];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Build HNSW index ---- */

void DBSCAN13::buildHNSW()
{
    int n = m_data.size();
    m_hnswGraph.resize(n);
    m_entryPoint.resize(1, 0);

    for (int i = 0; i < n; ++i) {
        m_hnswGraph[i].clear();

        // Simplified HNSW: connect to M nearest among previous nodes
        QVector<QPair<double, int>> dists;
        for (int j = 0; j < i; ++j) {
            double d = distance(i, j);
            dists.append(qMakePair(d, j));
        }

        std::sort(dists.begin(), dists.end());
        int maxConn = qMin(m_hnswM, dists.size());
        for (int k = 0; k < maxConn; ++k) {
            int j = dists[k].second;
            m_hnswGraph[i].append(j);
            m_hnswGraph[j].append(i);
        }
    }
}

/* ---- HNSW approximate k-NN search ---- */

QVector<int> DBSCAN13::hnswSearch(const QVector<double>& query, int k) const
{
    int n = m_data.size();
    if (n == 0) return QVector<int>();

    // Start from entry point, greedily expand
    QVector<QPair<double, int>> candidates;
    QVector<bool> visited(n, false);

    int cur = 0;
    for (int step = 0; step < 50; ++step) {
        if (visited[cur]) break;
        visited[cur] = true;

        double d = 0.0;
        int dim = qMin(query.size(), m_data[cur].size());
        for (int i = 0; i < dim; ++i) {
            double diff = query[i] - m_data[cur][i];
            d += diff * diff;
        }
        candidates.append(qMakePair(qSqrt(d), cur));

        // Expand to neighbors
        for (int nb : m_hnswGraph[cur]) {
            if (!visited[nb]) {
                visited[nb] = true;
                double nd = 0.0;
                int dd = qMin(query.size(), m_data[nb].size());
                for (int i = 0; i < dd; ++i) {
                    double diff = query[i] - m_data[nb][i];
                    nd += diff * diff;
                }
                candidates.append(qMakePair(qSqrt(nd), nb));
            }
        }

        // Move to best unvisited neighbor
        double bestDist = candidates.last().first;
        cur = candidates.last().second;
        for (int nb : m_hnswGraph[cur]) {
            if (!visited[nb]) { cur = nb; break; }
        }
    }

    std::sort(candidates.begin(), candidates.end());
    QVector<int> result;
    for (int i = 0; i < qMin(k, candidates.size()); ++i)
        result.append(candidates[i].second);
    return result;
}

/* ---- Compute local reachability density ---- */

double DBSCAN13::computeLRD(int idx) const
{
    QVector<int> neighbors = hnswSearch(m_data[idx], m_minPts);
    if (neighbors.isEmpty()) return 0.0;

    double reachSum = 0.0;
    for (int nb : neighbors) {
        double d = distance(idx, nb);
        reachSum += qMax(d, m_epsilon);
    }
    return neighbors.size() / reachSum;
}

/* ---- Compute adaptive epsilon ---- */

double DBSCAN13::computeAdaptiveEpsilon(int idx) const
{
    double lrd = computeLRD(idx);
    if (lrd <= 0.0) return m_epsilon;

    // Scale epsilon inversely with local density
    double densityFactor = 1.0 / (1.0 + lrd);
    return m_epsilon * (0.5 + densityFactor);
}

/* ---- Region query using HNSW ---- */

QVector<int> DBSCAN13::regionQuery(int idx, double eps) const
{
    QVector<int> result;
    int n = m_data.size();

    // Use HNSW graph for fast neighbor discovery
    QVector<int> candidates = hnswSearch(m_data[idx], qMax(m_minPts * 3, n));

    // Also check direct neighbors in HNSW graph
    QSet<int> checked;
    for (int c : candidates) checked.insert(c);
    for (int nb : m_hnswGraph[idx]) {
        if (!checked.contains(nb)) {
            candidates.append(nb);
            checked.insert(nb);
        }
    }

    for (int c : candidates) {
        if (distance(idx, c) <= eps)
            result.append(c);
    }
    return result;
}

/* ---- Expand cluster ---- */

void DBSCAN13::expandCluster(int idx, int clusterId, double eps)
{
    QVector<int> seeds = regionQuery(idx, eps);
    m_labels[idx].cluster = clusterId;
    m_labels[idx].visited = true;

    QVector<int> queue = seeds;
    while (!queue.isEmpty()) {
        int cur = queue.takeFirst();

        if (!m_labels[cur].visited) {
            m_labels[cur].visited = true;
            QVector<int> curNeighbors = regionQuery(cur, eps);

            if (curNeighbors.size() >= m_minPts) {
                for (int nb : curNeighbors) {
                    if (!m_labels[nb].visited || m_labels[nb].cluster == -1)
                        queue.append(nb);
                }
            }
        }

        if (m_labels[cur].cluster == -1) {
            m_labels[cur].cluster = clusterId;
        }
    }

    emit clusterExpanded(clusterId, seeds.size());
}

/* ---- Fit DBSCAN ---- */

bool DBSCAN13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_minPts) return false;

    m_data = data;
    m_labels.resize(n);
    m_clusterCount = 0;

    // Compute adaptive epsilon for each point
    for (int i = 0; i < n; ++i)
        m_labels[i].adaptiveEpsilon = computeAdaptiveEpsilon(i);

    // Build HNSW index
    buildHNSW();

    // Main DBSCAN loop
    for (int i = 0; i < n; ++i) {
        if (m_labels[i].visited) continue;

        double eps = m_labels[i].adaptiveEpsilon;
        QVector<int> neighbors = regionQuery(i, eps);

        if (neighbors.size() < m_minPts) {
            m_labels[i].cluster = -1;  // noise
            continue;
        }

        expandCluster(i, m_clusterCount, eps);
        m_clusterCount++;
    }

    int noise = 0;
    for (int i = 0; i < n; ++i)
        if (m_labels[i].cluster == -1) noise++;

    m_stats.numPoints = n;
    m_stats.numClusters = m_clusterCount;
    m_stats.numDimensions = (n > 0) ? data[0].size() : 0;
    m_stats.numNoise = noise;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_clusterCount, noise, timer.elapsed());
    return true;
}

/* ---- Labels ---- */

QVector<int> DBSCAN13::labels() const
{
    QVector<int> result(m_labels.size());
    for (int i = 0; i < m_labels.size(); ++i)
        result[i] = m_labels[i].cluster;
    return result;
}

/* ---- Adaptive epsilons ---- */

QVector<double> DBSCAN13::adaptiveEpsilons() const
{
    QVector<double> result(m_labels.size());
    for (int i = 0; i < m_labels.size(); ++i)
        result[i] = m_labels[i].adaptiveEpsilon;
    return result;
}

/* ---- Cluster count ---- */

int DBSCAN13::clusterCount() const { return m_clusterCount; }

/* ---- Reset ---- */

void DBSCAN13::resetStatistics()
{
    m_data.clear();
    m_labels.clear();
    m_hnswGraph.clear();
    m_entryPoint.clear();
    m_clusterCount = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
