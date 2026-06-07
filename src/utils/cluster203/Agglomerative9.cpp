/**
 * @file Agglomerative9.cpp
 * @brief Agglomerative9 实现
 *
 * 实现加权平均链接层次聚合聚类：距离矩阵构建、链接矩阵生成、动态树切割。
 */

#include "utils/cluster203/Agglomerative9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative9::Agglomerative9(QObject *parent) : QObject(parent) {}
Agglomerative9::~Agglomerative9() = default;

/* ---- Configuration ---- */

void Agglomerative9::setNumClusters(int k) { m_targetK = qMax(1, k); }
void Agglomerative9::setCutHeight(double height) { m_cutHeight = qMax(0.0, height); }

/* ---- Euclidean distance ---- */

double Agglomerative9::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Build pairwise distance matrix ---- */

QVector<QVector<double>> Agglomerative9::buildDistMatrix(const QVector<QVector<double>>& data)
{
    int n = data.size();
    QVector<QVector<double>> dm(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dm[i][j] = d;
            dm[j][i] = d;
        }
    return dm;
}

/* ---- Weighted-average linkage distance ---- */

double Agglomerative9::weightedAverageDist(int c1, int c2,
                                            const QVector<QVector<double>>& distMatrix,
                                            const QVector<int>& sizes) const
{
    int n = distMatrix.size();
    double totalWeight = 0.0;
    double weightedSum = 0.0;
    for (int i = 0; i < n; ++i) {
        if (sizes[i] <= 0) continue;
        for (int j = i + 1; j < n; ++j) {
            if (sizes[j] <= 0) continue;
            // Weighted by inverse cluster size
            double w = 1.0 / (sizes[i] * sizes[j]);
            weightedSum += distMatrix[i][j] * w;
            totalWeight += w;
        }
    }
    return (totalWeight > 0.0) ? weightedSum / totalWeight : 0.0;
}

/* ---- Build linkage matrix ---- */

QVector<QVector<double>> Agglomerative9::linkage(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n < 2) return {};

    QVector<QVector<double>> dm = buildDistMatrix(data);
    QVector<int> clusterId(n);
    for (int i = 0; i < n; ++i) clusterId[i] = i;
    QVector<int> sizes(n, 1);
    QVector<bool> active(n, true);

    QVector<QVector<double>> linkMat(n - 1, QVector<double>(4, 0.0));

    for (int step = 0; step < n - 1; ++step) {
        // Find closest pair among active clusters
        double bestDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < n + step; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n + step; ++j) {
                if (!active[j]) continue;
                if (i >= dm.size() || j >= dm.size()) continue;
                if (dm[i][j] < bestDist) {
                    bestDist = dm[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;

        int newId = n + step;
        int mergedSize = sizes[bestI] + sizes[bestJ];
        linkMat[step] = {static_cast<double>(bestI), static_cast<double>(bestJ),
                         bestDist, static_cast<double>(mergedSize)};

        // Extend distance matrix for new cluster
        dm.resize(newId + 1);
        for (auto& row : dm) row.resize(newId + 1, 0.0);
        sizes.resize(newId + 1);
        active.resize(newId + 1);

        // Compute weighted-average distance from new cluster to all active
        for (int k = 0; k < newId; ++k) {
            if (!active[k] || k == bestI || k == bestJ) continue;
            double si = sizes[bestI], sj = sizes[bestJ];
            double d = (si * dm[bestI][k] + sj * dm[bestJ][k]) / (si + sj);
            dm[newId][k] = d;
            dm[k][newId] = d;
        }

        sizes[newId] = mergedSize;
        active[newId] = true;
        active[bestI] = false;
        active[bestJ] = false;
    }

    return linkMat;
}

/* ---- Cut tree at fixed height ---- */

QVector<int> Agglomerative9::cutTree(const QVector<QVector<double>>& linkageMatrix,
                                      int n, double height) const
{
    QVector<int> labels(n + linkageMatrix.size(), -1);
    // Initially each point is its own cluster
    int nextLabel = 0;
    for (int i = 0; i < n; ++i) labels[i] = nextLabel++;

    for (int s = 0; s < linkageMatrix.size(); ++s) {
        int c1 = static_cast<int>(linkageMatrix[s][0]);
        int c2 = static_cast<int>(linkageMatrix[s][1]);
        double d = linkageMatrix[s][2];
        if (d > height) {
            // Don't merge: new cluster node gets fresh label
            labels[n + s] = nextLabel++;
        } else {
            // Merge: propagate label of c1
            int mergedLabel = labels[c1];
            labels[n + s] = mergedLabel;
            // Relabel all points in c2 to mergedLabel
            for (int i = 0; i < n; ++i) {
                // Trace membership through union-find-like labeling
                if (labels[i] == labels[c2]) labels[i] = mergedLabel;
            }
        }
    }

    // Compact labels
    labels.resize(n);
    QVector<int> compact(n, -1);
    QMap<int, int> remap;
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        if (!remap.contains(labels[i])) remap[labels[i]] = idx++;
        compact[i] = remap[labels[i]];
    }
    return compact;
}

/* ---- Dynamic tree cut ---- */

QVector<int> Agglomerative9::dynamicCut(const QVector<QVector<double>>& linkageMatrix, int n)
{
    if (linkageMatrix.isEmpty()) return {};
    // Compute adaptive threshold from merge heights
    double sumH = 0.0, maxH = 0.0;
    for (const auto& row : linkageMatrix) {
        sumH += row[2];
        if (row[2] > maxH) maxH = row[2];
    }
    double avgH = sumH / linkageMatrix.size();
    // Dynamic cut height: use mean + fraction of max
    double dynHeight = avgH + 0.3 * (maxH - avgH);
    return cutTree(linkageMatrix, n, dynHeight);
}

/* ---- Fit ---- */

QVector<int> Agglomerative9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    int n = data.size();
    if (n == 0) return {};

    QVector<QVector<double>> lm = linkage(data);
    QVector<int> labels;
    if (m_cutHeight > 0.0)
        labels = cutTree(lm, n, m_cutHeight);
    else
        labels = dynamicCut(lm, n);

    m_stats.totalOps++;
    m_stats.numPoints = n;
    m_stats.numClusters = (labels.isEmpty()) ? 0 :
        *std::max_element(labels.begin(), labels.end()) + 1;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_stats.numClusters, timer.elapsed());
    return labels;
}

/* ---- Reset ---- */

void Agglomerative9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
