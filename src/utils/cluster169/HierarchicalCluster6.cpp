/**
 * @file HierarchicalCluster6.cpp
 * @brief HierarchicalCluster6 实现
 *
 * 实现凝聚层次聚类：距离矩阵计算、贪心合并、树状图构建、动态切割。
 */

#include "utils/cluster169/HierarchicalCluster6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HierarchicalCluster6::HierarchicalCluster6(QObject *parent)
    : QObject(parent)
{
}

HierarchicalCluster6::~HierarchicalCluster6() = default;

/* ---- Configuration ---- */

void HierarchicalCluster6::setLinkage(Linkage method) { m_linkage = method; }
void HierarchicalCluster6::setCutHeight(double height) {
    m_cutHeight = height;
    m_useCutHeight = true;
}

/* ---- Distance ---- */

double HierarchicalCluster6::distance(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

QVector<QVector<double>> HierarchicalCluster6::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = distance(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- Update distances after merge ---- */

void HierarchicalCluster6::updateDistances(QVector<QVector<double>>& dist,
                                            int mergedI, int mergedJ,
                                            const QVector<int>& clusterSize) const
{
    int n = dist.size();
    /* Merge cluster J into cluster I using Lance-Williams formula */
    double ni = clusterSize[mergedI];
    double nj = clusterSize[mergedJ];

    for (int k = 0; k < n; ++k) {
        if (k == mergedI || k == mergedJ) continue;
        if (dist[mergedI][k] < 0) continue; /* inactive cluster */

        double dik = dist[mergedI][k];
        double djk = dist[mergedJ][k];
        double dij = dist[mergedI][mergedJ];
        double nk = clusterSize[k];
        double newDist = 0.0;

        switch (m_linkage) {
        case Single:
            newDist = qMin(dik, djk);
            break;
        case Complete:
            newDist = qMax(dik, djk);
            break;
        case Average:
            newDist = (ni * dik + nj * djk) / (ni + nj);
            break;
        case Ward:
            newDist = qSqrt(((ni + nk) * dik * dik + (nj + nk) * djk * djk
                             - nk * dij * dij) / (ni + nj + nk));
            break;
        }

        dist[mergedI][k] = newDist;
        dist[k][mergedI] = newDist;
    }

    /* Deactivate cluster J */
    for (int k = 0; k < n; ++k) {
        dist[mergedJ][k] = -1.0;
        dist[k][mergedJ] = -1.0;
    }
}

/* ---- Main clustering ---- */

QVector<int> HierarchicalCluster6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return QVector<int>();
    m_n = n;

    /* Distance matrix */
    QVector<QVector<double>> dist = computeDistanceMatrix(data);

    /* Active cluster tracking */
    QVector<int> clusterMap(n);
    for (int i = 0; i < n; ++i) clusterMap[i] = i;
    QVector<int> cSize(n, 1);
    QVector<bool> active(n, true);

    m_dendrogram.clear();
    m_dendrogram.resize(n - 1);

    /* Agglomerative loop */
    for (int step = 0; step < n - 1; ++step) {
        /* Find closest active pair */
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < 0) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        /* Record dendrogram node */
        m_dendrogram[step].left = bestI;
        m_dendrogram[step].right = bestJ;
        m_dendrogram[step].distance = minDist;
        m_dendrogram[step].size = cSize[bestI] + cSize[bestJ];

        /* Merge J into I */
        updateDistances(dist, bestI, bestJ, cSize);
        cSize[bestI] += cSize[bestJ];
        active[bestJ] = false;

        /* Remap cluster labels */
        int targetCluster = clusterMap[bestI];
        int srcCluster = clusterMap[bestJ];
        for (int k = 0; k < n; ++k)
            if (clusterMap[k] == srcCluster) clusterMap[k] = targetCluster;
    }

    /* Assign labels from dendrogram cut */
    int numClusters = 0;
    if (m_useCutHeight) {
        /* Count merges below cut height */
        QVector<int> rootCluster(n);
        for (int i = 0; i < n; ++i) rootCluster[i] = i;

        for (int s = 0; s < m_dendrogram.size(); ++s) {
            if (m_dendrogram[s].distance > m_cutHeight) break;
            int rootJ = rootCluster[m_dendrogram[s].right];
            int rootI = rootCluster[m_dendrogram[s].left];
            for (int k = 0; k < n; ++k)
                if (rootCluster[k] == rootJ) rootCluster[k] = rootI;
        }

        /* Assign sequential labels */
        QVector<int> uniqueRoots;
        for (int i = 0; i < n; ++i) {
            if (!uniqueRoots.contains(rootCluster[i]))
                uniqueRoots.append(rootCluster[i]);
        }
        numClusters = uniqueRoots.size();
        QVector<int> labels(n);
        for (int i = 0; i < n; ++i)
            labels[i] = uniqueRoots.indexOf(rootCluster[i]);
        clusterMap = labels;
    } else {
        clusterMap = cutTree(qMax(1, 2));
        numClusters = 2;
        for (int c : clusterMap) if (c + 1 > numClusters) numClusters = c + 1;
    }

    /* Update stats */
    m_stats.totalRuns++;
    m_stats.lastClusters = numClusters;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(numClusters);
    return clusterMap;
}

/* ---- Dendrogram access ---- */

QVector<HierarchicalCluster6::DendrogramNode> HierarchicalCluster6::dendrogram() const
{
    return m_dendrogram;
}

/* ---- Cut tree at specified cluster count ---- */

QVector<int> HierarchicalCluster6::cutTree(int numClusters) const
{
    int n = m_n;
    if (n == 0 || m_dendrogram.isEmpty()) return QVector<int>();

    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) labels[i] = i;

    /* Perform (n - numClusters) merges */
    int merges = qMax(0, n - numClusters);
    for (int s = 0; s < merges && s < m_dendrogram.size(); ++s) {
        int src = labels[m_dendrogram[s].right];
        int dst = labels[m_dendrogram[s].left];
        for (int k = 0; k < n; ++k)
            if (labels[k] == src) labels[k] = dst;
    }

    /* Relabel to sequential 0..K-1 */
    QVector<int> unique;
    for (int l : labels)
        if (!unique.contains(l)) unique.append(l);

    QVector<int> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = unique.indexOf(labels[i]);
    return result;
}

/* ---- Statistics ---- */

void HierarchicalCluster6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
