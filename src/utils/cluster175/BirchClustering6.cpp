/**
 * @file BirchClustering6.cpp
 * @brief BirchClustering6 实现
 *
 * 实现BIRCH聚类：CF树构建、自适应阈值、子簇合并、内存预算。
 */

#include "utils/cluster175/BirchClustering6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- CFEntry helpers ---- */

QVector<double> BirchClustering6::CFEntry::centroid() const
{
    if (n == 0) return {};
    QVector<double> c(ls.size());
    for (int i = 0; i < ls.size(); ++i)
        c[i] = ls[i] / n;
    return c;
}

double BirchClustering6::CFEntry::radius() const
{
    if (n <= 1) return 0.0;
    double r = 0.0;
    for (int i = 0; i < ls.size(); ++i)
        r += ss[i] / n - (ls[i] / n) * (ls[i] / n);
    return qSqrt(qMax(0.0, r));
}

/* ---- Construction / Destruction ---- */

BirchClustering6::BirchClustering6(QObject *parent)
    : QObject(parent)
{
}

BirchClustering6::~BirchClustering6() { deleteTree(m_root); }

/* ---- Configuration ---- */

void BirchClustering6::setThreshold(double t) { m_threshold = qMax(0.01, t); }
void BirchClustering6::setBranchingFactor(int b) { m_branching = qMax(2, b); }
void BirchClustering6::setNumClusters(int k) { m_numClusters = qMax(2, k); }
void BirchClustering6::setMemoryBudget(int maxNodes) { m_maxNodes = qMax(100, maxNodes); }

/* ---- Distance / merge ---- */

double BirchClustering6::euclideanDist(const QVector<double>& a,
                                       const QVector<double>& b)
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

BirchClustering6::CFEntry BirchClustering6::mergeCF(const CFEntry& a,
                                                     const CFEntry& b)
{
    CFEntry result;
    result.n = a.n + b.n;
    result.ls.resize(a.ls.size());
    result.ss.resize(a.ss.size());
    for (int i = 0; i < a.ls.size(); ++i) {
        result.ls[i] = a.ls[i] + b.ls[i];
        result.ss[i] = a.ss[i] + b.ss[i];
    }
    return result;
}

/* ---- Tree management ---- */

void BirchClustering6::deleteTree(CFNode* node)
{
    if (!node) return;
    for (auto* child : node->children)
        deleteTree(child);
    delete node;
}

void BirchClustering6::insertPoint(const QVector<double>& point)
{
    /* Find closest leaf entry */
    int bestIdx = 0;
    double bestDist = 1e18;
    for (int i = 0; i < m_leafEntries.size(); ++i) {
        auto c = m_leafEntries[i].centroid();
        double d = euclideanDist(point, c);
        if (d < bestDist) { bestDist = d; bestIdx = i; }
    }

    /* Check threshold: absorb or create new subcluster */
    if (!m_leafEntries.isEmpty() && bestDist <= m_threshold) {
        auto& e = m_leafEntries[bestIdx];
        e.n++;
        for (int i = 0; i < m_dims; ++i) {
            e.ls[i] += point[i];
            e.ss[i] += point[i] * point[i];
        }
    } else {
        CFEntry entry;
        entry.n = 1;
        entry.ls = point;
        entry.ss.resize(m_dims);
        for (int i = 0; i < m_dims; ++i)
            entry.ss[i] = point[i] * point[i];
        m_leafEntries.append(entry);
    }
}

void BirchClustering6::mergeLeafEntries()
{
    /* Merge closest pair until count <= branching factor or no close pairs */
    while (m_leafEntries.size() > m_maxNodes) {
        double minDist = 1e18;
        int mi = 0, mj = 1;
        for (int i = 0; i < m_leafEntries.size(); ++i) {
            auto ci = m_leafEntries[i].centroid();
            for (int j = i + 1; j < m_leafEntries.size(); ++j) {
                auto cj = m_leafEntries[j].centroid();
                double d = euclideanDist(ci, cj);
                if (d < minDist) { minDist = d; mi = i; mj = j; }
            }
        }
        m_leafEntries[mi] = mergeCF(m_leafEntries[mi], m_leafEntries[mj]);
        m_leafEntries.removeAt(mj);
    }
}

void BirchClustering6::rebuildTree()
{
    /* Adaptive threshold: increase if too many leaf entries */
    if (m_leafEntries.size() > m_branching * 10) {
        m_threshold *= 1.5;
        emit treeRebuilt(m_leafEntries.size());
    }
}

/* ---- Main fit ---- */

QVector<int> BirchClustering6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    m_dims = data[0].size();
    m_leafEntries.clear();

    /* Phase 1: Build CF tree by inserting points */
    for (int i = 0; i < n; ++i)
        insertPoint(data[i]);

    /* Phase 2: Condense - merge close subclusters */
    mergeLeafEntries();
    rebuildTree();

    /* Phase 3: Global clustering via simple k-means on subcluster centroids */
    int sc = m_leafEntries.size();
    int k = qMin(m_numClusters, sc);

    /* Initialize centroids from subcluster centroids */
    QVector<QVector<double>> centroids(k);
    for (int i = 0; i < k; ++i)
        centroids[i] = m_leafEntries[i * qMax(1, sc / k)].centroid();

    /* Assign each subcluster to nearest centroid */
    QVector<int> scLabels(sc, 0);
    for (int iter = 0; iter < 20; ++iter) {
        /* Assign */
        for (int i = 0; i < sc; ++i) {
            double minD = 1e18;
            for (int j = 0; j < k; ++j) {
                double d = euclideanDist(m_leafEntries[i].centroid(), centroids[j]);
                if (d < minD) { minD = d; scLabels[i] = j; }
            }
        }
        /* Update centroids */
        for (int j = 0; j < k; ++j) {
            QVector<double> sum(m_dims, 0.0);
            int cnt = 0;
            for (int i = 0; i < sc; ++i) {
                if (scLabels[i] == j) {
                    auto c = m_leafEntries[i].centroid();
                    for (int d = 0; d < m_dims; ++d)
                        sum[d] += c[d] * m_leafEntries[i].n;
                    cnt += m_leafEntries[i].n;
                }
            }
            if (cnt > 0) {
                for (int d = 0; d < m_dims; ++d)
                    sum[d] /= cnt;
                centroids[j] = sum;
            }
        }
    }

    /* Map each data point to its cluster label */
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        double minD = 1e18;
        for (int j = 0; j < k; ++j) {
            double d = euclideanDist(data[i], centroids[j]);
            if (d < minD) { minD = d; m_labels[i] = j; }
        }
    }

    m_stats.totalRuns++;
    m_stats.numClusters = k;
    m_stats.numSamples = n;
    m_stats.treeNodes = sc;
    m_stats.threshold = m_threshold;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(k, sc);
    return m_labels;
}

QVector<BirchClustering6::CFEntry> BirchClustering6::subclusters() const
{
    return m_leafEntries;
}

void BirchClustering6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
