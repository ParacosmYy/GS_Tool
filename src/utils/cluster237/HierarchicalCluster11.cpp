/**
 * @file HierarchicalCluster11.cpp
 * @brief HierarchicalCluster11 实现
 *
 * 实现层次聚类：Ward链接、Lance-Williams相异性更新与最小方差合并。
 */

#include "utils/cluster237/HierarchicalCluster11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HierarchicalCluster11::HierarchicalCluster11(QObject *parent) : QObject(parent) {}
HierarchicalCluster11::~HierarchicalCluster11() = default;

/* ---- Configuration ---- */

void HierarchicalCluster11::setNumClusters(int k) { m_k = qMax(2, k); }

/* ---- Squared Euclidean distance ---- */

double HierarchicalCluster11::sqDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Lance-Williams update for Ward linkage ---- */

double HierarchicalCluster11::lanceWilliamsUpdate(double dAi, double dBi, double dAB,
                                                   int sizeA, int sizeB, int sizeI) const
{
    // Ward: alpha_i = (|A|+|I|)/(|A|+|B|+|I|), beta = -|I|/(|A|+|B|+|I|)
    int total = sizeA + sizeB + sizeI;
    double alphaA = static_cast<double>(sizeA + sizeI) / total;
    double alphaB = static_cast<double>(sizeB + sizeI) / total;
    double beta = -static_cast<double>(sizeI) / total;
    return alphaA * dAi + alphaB * dBi + beta * dAB;
}

/* ---- Union-Find helpers ---- */

int HierarchicalCluster11::ufFind(QVector<int>& parent, int i) const
{
    while (parent[i] != i) {
        parent[i] = parent[parent[i]];
        i = parent[i];
    }
    return i;
}

void HierarchicalCluster11::ufUnion(QVector<int>& parent, QVector<int>& sz, int a, int b)
{
    if (sz[a] < sz[b]) { parent[a] = b; sz[b] += sz[a]; }
    else { parent[b] = a; sz[a] += sz[b]; }
}

/* ---- Fit ---- */

QVector<int> HierarchicalCluster11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();
    if (n < m_k) return {};

    m_stats.numPoints = n;
    m_stats.numDimensions = data[0].size();
    m_history.clear();

    // Active cluster flags and sizes
    QVector<bool> active(n, true);
    QVector<int> clusterSize(n, 1);

    // Distance matrix (upper triangle stored as full)
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = sqDist(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }

    // Map from cluster index to union-find root
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    int numActive = n;
    int step = 0;

    while (numActive > m_k) {
        // Find minimum distance pair
        double minDist = std::numeric_limits<double>::max();
        int mergeI = -1, mergeJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        // Record merge step
        MergeStep ms;
        ms.clusterA = mergeI;
        ms.clusterB = mergeJ;
        ms.distance = minDist;
        ms.newSize = clusterSize[mergeI] + clusterSize[mergeJ];
        m_history.append(ms);

        // Update distances via Lance-Williams
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeI || k == mergeJ) continue;
            dist[mergeI][k] = lanceWilliamsUpdate(
                dist[mergeI][k], dist[mergeJ][k], minDist,
                clusterSize[mergeI], clusterSize[mergeJ], clusterSize[k]);
            dist[k][mergeI] = dist[mergeI][k];
        }

        // Merge clusters
        clusterSize[mergeI] += clusterSize[mergeJ];
        active[mergeJ] = false;
        ufUnion(parent, clusterSize, mergeI, mergeJ);

        numActive--;
        step++;

        emit mergeCompleted(step, mergeI, mergeJ, minDist);
    }

    // Assign labels via union-find roots
    m_labels.resize(n);
    QVector<int> rootMap(n, -1);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        int root = ufFind(parent, i);
        if (rootMap[root] < 0) rootMap[root] = label++;
        m_labels[i] = rootMap[root];
    }

    m_stats.numClusters = numActive;
    m_stats.numMerges = step;
    m_stats.finalDistance = m_history.isEmpty() ? 0.0 : m_history.last().distance;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_stats.numClusters, step, timer.elapsed());
    return m_labels;
}

/* ---- Cut at distance threshold ---- */

QVector<int> HierarchicalCluster11::cutAtDistance(double threshold) const
{
    int n = m_data.size();
    if (n == 0) return {};

    QVector<int> parent(n);
    QVector<int> sz(n, 1);
    for (int i = 0; i < n; ++i) parent[i] = i;

    for (const auto& ms : m_history) {
        if (ms.distance > threshold) break;
        int rA = ufFind(const_cast<QVector<int>&>(parent), ms.clusterA);
        int rB = ufFind(const_cast<QVector<int>&>(parent), ms.clusterB);
        if (rA != rB) ufUnion(const_cast<QVector<int>&>(parent),
                               const_cast<QVector<int>&>(sz), rA, rB);
    }

    QVector<int> result(n);
    QVector<int> rootMap(n, -1);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        int root = ufFind(const_cast<QVector<int>&>(parent), i);
        if (rootMap[root] < 0) rootMap[root] = label++;
        result[i] = rootMap[root];
    }
    return result;
}

/* ---- Accessors ---- */

QVector<int> HierarchicalCluster11::labels() const { return m_labels; }
QVector<HierarchicalCluster11::MergeStep> HierarchicalCluster11::mergeHistory() const { return m_history; }

/* ---- Cophenetic correlation ---- */

double HierarchicalCluster11::copheneticCorrelation(const QVector<QVector<double>>& origDist) const
{
    int n = origDist.size();
    if (n < 2) return 0.0;

    // Build cophenetic distance matrix from merge history
    QVector<QVector<double>> coph(n, QVector<double>(n, 0.0));
    QVector<int> parent(n), sz(n, 1);
    for (int i = 0; i < n; ++i) parent[i] = i;

    for (const auto& ms : m_history) {
        // All pairs across clusters A and B have cophenetic distance = merge distance
        QVector<int> membersA, membersB;
        for (int i = 0; i < n; ++i)
            if (ufFind(const_cast<QVector<int>&>(parent), i) ==
                ufFind(const_cast<QVector<int>&>(parent), ms.clusterA))
                membersA.append(i);

        // Temporarily merge for next iterations
        int rA = ufFind(const_cast<QVector<int>&>(parent), ms.clusterA);
        int rB = ufFind(const_cast<QVector<int>&>(parent), ms.clusterB);
        if (rA != rB) ufUnion(const_cast<QVector<int>&>(parent),
                               const_cast<QVector<int>&>(sz), rA, rB);

        for (int i = 0; i < n; ++i) {
            if (ufFind(const_cast<QVector<int>&>(parent), i) ==
                ufFind(const_cast<QVector<int>&>(parent), ms.clusterA)) {
                if (!membersA.contains(i)) membersB.append(i);
            }
        }

        for (int a : membersA)
            for (int b : membersB) {
                if (a < b) { coph[a][b] = ms.distance; coph[b][a] = ms.distance; }
            }
    }

    // Pearson correlation between original and cophenetic distances
    double sumXY = 0.0, sumX = 0.0, sumY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    int count = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double x = origDist[i][j];
            double y = coph[i][j];
            sumX += x; sumY += y;
            sumX2 += x * x; sumY2 += y * y;
            sumXY += x * y;
            count++;
        }

    if (count == 0) return 0.0;
    double num = sumXY - sumX * sumY / count;
    double den = qSqrt((sumX2 - sumX * sumX / count) * (sumY2 - sumY * sumY / count));
    return (den < 1e-15) ? 0.0 : num / den;
}

/* ---- Reset ---- */

void HierarchicalCluster11::resetStatistics()
{
    m_data.clear(); m_labels.clear(); m_history.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
