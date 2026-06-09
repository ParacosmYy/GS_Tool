/**
 * @file HierarchicalCluster12.cpp
 * @brief HierarchicalCluster12 实现
 *
 * 实现层次聚类：UPGMA非加权平均合并与共表距离矩阵构建有根树。
 */

#include "utils/cluster251/HierarchicalCluster12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

HierarchicalCluster12::HierarchicalCluster12(QObject *parent)
    : QObject(parent) {}
HierarchicalCluster12::~HierarchicalCluster12() = default;

/* ---- Configuration ---- */

void HierarchicalCluster12::setTargetClusters(int k)
{
    m_targetK = qMax(1, k);
}

/* ---- Euclidean distance ---- */

double HierarchicalCluster12::euclidean(
    const QVector<QVector<double>>& data, int i, int j) const
{
    double sum = 0.0;
    int d = data[i].size();
    for (int dim = 0; dim < d; ++dim) {
        double diff = data[i][dim] - data[j][dim];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Initialize pairwise distance matrix ---- */

void HierarchicalCluster12::initDistanceMatrix(
    const QVector<QVector<double>>& data)
{
    m_distMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_distMatrix[i].resize(m_n, 0.0);
        for (int j = i + 1; j < m_n; ++j) {
            double d = euclidean(data, i, j);
            m_distMatrix[i][j] = d;
            m_distMatrix[j][i] = d;
        }
    }
}

/* ---- UPGMA merge step ---- */

void HierarchicalCluster12::upgmaMerge(int a, int b,
    const QVector<int>& sizes, QVector<int>& active)
{
    int sizeA = sizes[a];
    int sizeB = sizes[b];
    int totalSize = sizeA + sizeB;

    // Update distances using UPGMA unweighted average formula:
    // d(new, k) = (sizeA * d(a,k) + sizeB * d(b,k)) / (sizeA + sizeB)
    for (int k = 0; k < m_n; ++k) {
        if (!active[k] || k == a || k == b) continue;
        double newDist = (sizeA * m_distMatrix[a][k]
                          + sizeB * m_distMatrix[b][k]) / totalSize;
        m_distMatrix[a][k] = newDist;
        m_distMatrix[k][a] = newDist;
    }

    // Deactivate cluster b
    active[b] = 0;
    Q_UNUSED(totalSize);
}

/* ---- Main fit ---- */

QVector<int> HierarchicalCluster12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return {};
    m_targetK = qMin(m_targetK, m_n);

    initDistanceMatrix(data);

    // Each sample starts as its own cluster
    QVector<int> active(m_n, 1);
    QVector<int> sizes(m_n, 1);
    // Label map: maps original index to merged cluster index
    QVector<int> labelMap(m_n);
    for (int i = 0; i < m_n; ++i) labelMap[i] = i;

    m_merges.clear();
    int numActive = m_n;

    // Agglomerative loop: merge until targetK clusters remain
    while (numActive > m_targetK) {
        // Find closest pair of active clusters
        double minDist = std::numeric_limits<double>::max();
        int bestA = -1, bestB = -1;

        for (int i = 0; i < m_n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < m_n; ++j) {
                if (!active[j]) continue;
                if (m_distMatrix[i][j] < minDist) {
                    minDist = m_distMatrix[i][j];
                    bestA = i;
                    bestB = j;
                }
            }
        }

        if (bestA < 0) break;

        // Record merge event
        MergeEvent ev;
        ev.clusterA = bestA;
        ev.clusterB = bestB;
        ev.distance = minDist;
        ev.newSize = sizes[bestA] + sizes[bestB];
        m_merges.append(ev);

        // Perform UPGMA merge: keep bestA, deactivate bestB
        upgmaMerge(bestA, bestB, sizes, active);

        // Update cluster size
        sizes[bestA] += sizes[bestB];

        // Relabel: all points in bestB now belong to bestA's cluster
        for (int i = 0; i < m_n; ++i) {
            if (labelMap[i] == bestB) labelMap[i] = bestA;
        }

        numActive--;
    }

    // Assign final labels 0..k-1
    QVector<int> labels(m_n, -1);
    int labelIdx = 0;
    for (int i = 0; i < m_n; ++i) {
        if (!active[i]) continue;
        for (int j = 0; j < m_n; ++j) {
            if (labelMap[j] == i) labels[j] = labelIdx;
        }
        labelIdx++;
    }

    m_stats.numSamples = m_n;
    m_stats.numClusters = m_targetK;
    m_stats.numMerges = m_merges.size();
    m_stats.totalOps++;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_targetK, m_stats.copheneticCorrelation, elapsed);
    return labels;
}

/* ---- Dendrogram access ---- */

QVector<HierarchicalCluster12::MergeEvent> HierarchicalCluster12::dendrogram() const
{
    return m_merges;
}

/* ---- Cophenetic distance matrix ---- */

QVector<QVector<double>> HierarchicalCluster12::copheneticMatrix() const
{
    // Initialize with merge distances
    QVector<QVector<double>> coph(m_n, QVector<double>(m_n, 0.0));

    // Track which original indices belong to each cluster
    QVector<QVector<int>> members(m_n);
    for (int i = 0; i < m_n; ++i) members[i].append(i);

    // Replay merges to fill cophenetic distances
    QVector<int> clusterActive(m_n, 1);
    for (const auto& merge : m_merges) {
        // All pairs (a_member, b_member) have cophenetic = merge.distance
        for (int a : members[merge.clusterA]) {
            for (int b : members[merge.clusterB]) {
                coph[a][b] = merge.distance;
                coph[b][a] = merge.distance;
            }
        }
        // Merge members
        members[merge.clusterA].append(members[merge.clusterB]);
        members[merge.clusterB].clear();
    }

    return coph;
}

/* ---- Cophenetic correlation coefficient ---- */

double HierarchicalCluster12::copheneticCorrelation(
    const QVector<QVector<double>>& originalDist) const
{
    if (m_n < 2) return 0.0;

    QVector<QVector<double>> coph = copheneticMatrix();

    // Collect upper-triangle pairs
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    int count = 0;

    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            double x = originalDist[i][j];
            double y = coph[i][j];
            sumX += x;
            sumY += y;
            sumXY += x * y;
            sumX2 += x * x;
            sumY2 += y * y;
            count++;
        }
    }

    if (count == 0) return 0.0;
    double denom = qSqrt((count * sumX2 - sumX * sumX)
                         * (count * sumY2 - sumY * sumY));
    if (qFuzzyIsNull(denom)) return 0.0;

    double corr = (count * sumXY - sumX * sumY) / denom;
    m_stats.copheneticCorrelation = corr;
    return corr;
}

/* ---- Reset ---- */

void HierarchicalCluster12::resetStatistics()
{
    m_merges.clear();
    m_distMatrix.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
