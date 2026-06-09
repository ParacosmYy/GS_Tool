/**
 * @file DBSCAN15.cpp
 * @brief DBSCAN15 实现
 *
 * 实现HDBSCAN层次密度聚类：互达距离与最小生成树压缩层次提取。
 */

#include "utils/cluster256/DBSCAN15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

DBSCAN15::DBSCAN15(QObject *parent)
    : QObject(parent) {}
DBSCAN15::~DBSCAN15() = default;

/* ---- Configuration ---- */

void DBSCAN15::setMinClusterSize(int size) { m_minClusterSize = qMax(2, size); }
void DBSCAN15::setMinSamples(int samples) { m_minSamples = qMax(2, samples); }

/* ---- Pairwise distance ---- */

double DBSCAN15::distance(int i, int j) const
{
    double d = 0.0;
    for (int dim = 0; dim < m_dims; ++dim) {
        double diff = m_data[i][dim] - m_data[j][dim];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Core distances ---- */

void DBSCAN15::computeCoreDistances()
{
    m_coreDist.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        QVector<double> dists;
        dists.reserve(m_n - 1);
        for (int j = 0; j < m_n; ++j) {
            if (i != j) dists.append(distance(i, j));
        }
        std::sort(dists.begin(), dists.end());
        // core distance = distance to the minSamples-th nearest neighbor
        m_coreDist[i] = (m_minSamples - 1 < dists.size())
                            ? dists[m_minSamples - 1]
                            : std::numeric_limits<double>::infinity();
    }
}

/* ---- Mutual reachability ---- */

double DBSCAN15::mutualReachability(int i, int j) const
{
    double d = distance(i, j);
    return qMax({m_coreDist[i], m_coreDist[j], d});
}

/* ---- Build MST via Prim's ---- */

void DBSCAN15::buildMST()
{
    if (m_n == 0) return;
    m_mstEdges.clear();
    m_mstEdges.reserve(m_n - 1);

    QVector<bool> inMST(m_n, false);
    QVector<double> key(m_n, std::numeric_limits<double>::max());
    QVector<int> parent(m_n, -1);

    key[0] = 0.0;
    for (int iter = 0; iter < m_n; ++iter) {
        // Pick minimum key vertex not in MST
        int u = -1;
        double minKey = std::numeric_limits<double>::max();
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }
        }
        if (u < 0) break;
        inMST[u] = true;
        if (parent[u] >= 0) {
            m_mstEdges.append({parent[u], u, key[u]});
        }
        // Update keys
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v]) {
                double mr = mutualReachability(u, v);
                if (mr < key[v]) {
                    key[v] = mr;
                    parent[v] = u;
                }
            }
        }
    }
}

/* ---- Build condensed tree ---- */

void DBSCAN15::buildCondensedTree()
{
    m_condensed.clear();
    // Sort MST edges by weight ascending for hierarchy
    QVector<Edge> sortedEdges = m_mstEdges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const Edge& a, const Edge& b) { return a.weight < b.weight; });

    // Union-Find for cluster tracking
    QVector<int> ufParent(m_n);
    QVector<int> ufRank(m_n, 0);
    QVector<int> ufSize(m_n, 1);
    for (int i = 0; i < m_n; ++i) ufParent[i] = i;

    auto findSet = [&ufParent](int x) {
        while (ufParent[x] != x) {
            ufParent[x] = ufParent[ufParent[x]];
            x = ufParent[x];
        }
        return x;
    };

    int nextClusterId = m_n;
    // Track lambda for each merged cluster
    QVector<double> birthLambda(m_n + sortedEdges.size(), 0.0);

    for (const Edge& e : sortedEdges) {
        int ru = findSet(e.u);
        int rv = findSet(e.v);
        if (ru == rv) continue;

        double lambda = (e.weight > 0.0) ? 1.0 / e.weight : std::numeric_limits<double>::infinity();

        // Record condensed node for smaller cluster
        if (ufSize[ru] >= m_minClusterSize)
            m_condensed.append({nextClusterId, ru, lambda, ufSize[ru]});
        if (ufSize[rv] >= m_minClusterSize)
            m_condensed.append({nextClusterId, rv, lambda, ufSize[rv]});

        // Union by rank
        if (ufRank[ru] < ufRank[rv]) { int t = ru; ru = rv; rv = t; }
        ufParent[rv] = ru;
        ufSize[ru] += ufSize[rv];
        if (ufRank[ru] == ufRank[rv]) ufRank[ru]++;
        birthLambda[nextClusterId] = lambda;
        nextClusterId++;
    }
}

/* ---- Extract clusters ---- */

void DBSCAN15::extractClusters()
{
    m_labels.resize(m_n);
    m_probs.resize(m_n, 1.0);
    std::fill(m_labels.begin(), m_labels.end(), -1);

    if (m_condensed.isEmpty()) {
        // All points are noise if no condensed tree
        return;
    }

    // Find root cluster (last parent in condensed tree)
    int rootCluster = m_condensed.last().parent;

    // Simple extraction: assign points based on last stable split
    // Use MST-connected components at appropriate density level
    QVector<int> component(m_n);
    for (int i = 0; i < m_n; ++i) component[i] = i;

    // Sort edges by weight descending; cut at clusters
    QVector<Edge> sortedEdges = m_mstEdges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const Edge& a, const Edge& b) { return a.weight > b.weight; });

    // Determine cut threshold: use median of MST edge weights
    double cutThreshold = 0.0;
    if (!sortedEdges.isEmpty()) {
        int mid = sortedEdges.size() / 2;
        cutThreshold = sortedEdges[mid].weight;
    }

    // Remove high-weight edges (cut MST)
    QVector<int> ufParent(m_n);
    QVector<int> ufRank(m_n, 0);
    for (int i = 0; i < m_n; ++i) ufParent[i] = i;

    auto find = [&ufParent](int x) {
        while (ufParent[x] != x) { ufParent[x] = ufParent[ufParent[x]]; x = ufParent[x]; }
        return x;
    };

    for (const Edge& e : m_mstEdges) {
        if (e.weight <= cutThreshold) {
            int ru = find(e.u), rv = find(e.v);
            if (ru != rv) {
                if (ufRank[ru] < ufRank[rv]) { int t = ru; ru = rv; rv = t; }
                ufParent[rv] = ru;
                if (ufRank[ru] == ufRank[rv]) ufRank[ru]++;
            }
        }
    }

    // Map components to cluster IDs
    QMap<int, int> compToCluster;
    int clusterId = 0;
    for (int i = 0; i < m_n; ++i) {
        int root = find(i);
        if (!compToCluster.contains(root))
            compToCluster[root] = clusterId++;
        m_labels[i] = compToCluster[root];
    }

    // Mark small components as noise
    QVector<int> clusterSizes(clusterId, 0);
    for (int i = 0; i < m_n; ++i)
        if (m_labels[i] >= 0) clusterSizes[m_labels[i]]++;
    for (int i = 0; i < m_n; ++i) {
        if (m_labels[i] >= 0 && clusterSizes[m_labels[i]] < m_minClusterSize) {
            m_labels[i] = -1;
            m_probs[i] = 0.0;
        }
    }
}

/* ---- Main fit ---- */

bool DBSCAN15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < 2) return false;
    m_data = data;
    m_n = data.size();
    m_dims = data[0].size();

    computeCoreDistances();
    buildMST();
    buildCondensedTree();
    extractClusters();

    // Count clusters and noise
    int noiseCount = 0;
    QSet<int> uniqueClusters;
    for (int i = 0; i < m_n; ++i) {
        if (m_labels[i] < 0) noiseCount++;
        else uniqueClusters.insert(m_labels[i]);
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = m_n;
    m_stats.numClusters = uniqueClusters.size();
    m_stats.numNoise = noiseCount;
    m_stats.numEdges = m_mstEdges.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_stats.numClusters, noiseCount, elapsed);
    return true;
}

/* ---- Accessors ---- */

QVector<int> DBSCAN15::labels() const { return m_labels; }
QVector<double> DBSCAN15::probabilities() const { return m_probs; }
QVector<DBSCAN15::CondensedNode> DBSCAN15::condensedTree() const { return m_condensed; }

/* ---- Reset ---- */

void DBSCAN15::resetStatistics()
{
    m_data.clear();
    m_coreDist.clear();
    m_labels.clear();
    m_probs.clear();
    m_mstEdges.clear();
    m_condensed.clear();
    m_n = 0;
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
