/**
 * @file Agglomerative10.cpp
 * @brief Agglomerative10 实现
 *
 * 实现Ward最小方差凝聚聚类：距离矩阵维护、贪心合并、动态树切割。
 */

#include "utils/cluster217/Agglomerative10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative10::Agglomerative10(QObject *parent) : QObject(parent) {}
Agglomerative10::~Agglomerative10() = default;

/* ---- Configuration ---- */

void Agglomerative10::setParameters(int targetClusters, double heightThreshold)
{
    m_targetClusters = qMax(0, targetClusters);
    m_heightThreshold = qMax(0.0, heightThreshold);
}

/* ---- Squared Euclidean distance ---- */

double Agglomerative10::squaredDist(const QVector<double>& a,
                                     const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- Centroid of point set ---- */

QVector<double> Agglomerative10::centroid(const QVector<QVector<double>>& pts) const
{
    int n = pts.size();
    if (n == 0) return QVector<double>(m_dim, 0.0);
    QVector<double> c(m_dim, 0.0);
    for (int i = 0; i < n; ++i)
        for (int d = 0; d < m_dim; ++d)
            c[d] += pts[i][d];
    for (int d = 0; d < m_dim; ++d)
        c[d] /= n;
    return c;
}

/* ---- Ward linkage distance ---- */

double Agglomerative10::wardLinkage(int nA, const QVector<double>& cA,
                                     int nB, const QVector<double>& cB) const
{
    // Ward's minimum variance: |A|*|B|/(|A|+|B|) * ||cA - cB||^2
    double dist = squaredDist(cA, cB);
    return (double(nA) * double(nB) / double(nA + nB)) * dist;
}

/* ---- Ward distance between point sets (public) ---- */

double Agglomerative10::wardDistance(const QVector<QVector<double>>& a,
                                      const QVector<QVector<double>>& b) const
{
    if (a.isEmpty() || b.isEmpty()) return 0.0;
    int dim = a[0].size();
    QVector<double> cA(a.size() > 0 ? dim : 0, 0.0);
    QVector<double> cB(b.size() > 0 ? dim : 0, 0.0);
    for (int i = 0; i < a.size(); ++i)
        for (int d = 0; d < dim; ++d) cA[d] += a[i][d];
    for (int d = 0; d < dim; ++d) cA[d] /= a.size();
    for (int i = 0; i < b.size(); ++i)
        for (int d = 0; d < dim; ++d) cB[d] += b[i][d];
    for (int d = 0; d < dim; ++d) cB[d] /= b.size();
    return wardLinkage(a.size(), cA, b.size(), cB);
}

/* ---- Fit ---- */

void Agglomerative10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return;
    m_dim = data[0].size();
    m_merges.clear();

    // Each point starts as its own cluster
    QVector<int> clusterId(n);
    QVector<int> clusterSize(n, 1);
    QVector<QVector<double>> centroids(n);
    QVector<bool> active(n, true);
    for (int i = 0; i < n; ++i) {
        clusterId[i] = i;
        centroids[i] = data[i];
    }

    // Distance matrix (upper triangle stored flat)
    // Use QVector of QVector for simplicity
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[i][j] = wardLinkage(1, data[i], 1, data[j]);
        }
    }

    int nextId = n;
    int numActive = n;
    int targetK = m_targetClusters > 0 ? qMin(m_targetClusters, n) : 1;

    while (numActive > targetK) {
        // Find minimum distance pair
        double minDist = std::numeric_limits<double>::max();
        int minI = -1, minJ = -1;
        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    minI = i;
                    minJ = j;
                }
            }
        }
        if (minI < 0) break;

        // Check height threshold for auto-cut
        if (m_targetClusters == 0 && m_heightThreshold > 0.0
            && minDist > m_heightThreshold) {
            break;
        }

        // Record merge
        MergeRecord rec;
        rec.clusterA = clusterId[minI];
        rec.clusterB = clusterId[minJ];
        rec.distance = minDist;
        rec.newSize = clusterSize[minI] + clusterSize[minJ];
        m_merges.append(rec);

        // Update centroid via weighted average
        int sA = clusterSize[minI], sB = clusterSize[minJ];
        int sNew = sA + sB;
        for (int d = 0; d < m_dim; ++d) {
            centroids[minI][d] = (sA * centroids[minI][d]
                                  + sB * centroids[minJ][d]) / sNew;
        }
        clusterSize[minI] = sNew;
        clusterId[minI] = nextId++;

        // Update distances (Lance-Williams for Ward)
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == minI || k == minJ) continue;
            double dAI = dist[qMin(minI, k)][qMax(minI, k)];
            double dBJ = dist[qMin(minJ, k)][qMax(minJ, k)];
            double dIJ = minDist;
            // Ward update formula
            double newDist = qSqrt(
                ((sA + clusterSize[k]) * dAI * dAI
                 + (sB + clusterSize[k]) * dBJ * dBJ
                 - clusterSize[k] * dIJ * dIJ) / (sNew + clusterSize[k]));
            dist[qMin(minI, k)][qMax(minI, k)] = newDist;
        }

        active[minJ] = false;
        numActive--;
    }

    // Assign labels from active clusters
    m_labels.resize(n);
    QVector<int> activeIds;
    for (int i = 0; i < n; ++i)
        if (active[i]) activeIds.append(i);

    for (int i = 0; i < n; ++i) {
        int best = 0;
        double bestD = std::numeric_limits<double>::max();
        for (int c = 0; c < activeIds.size(); ++c) {
            double d = squaredDist(data[i], centroids[activeIds[c]]);
            if (d < bestD) { bestD = d; best = c; }
        }
        m_labels[i] = best;
    }

    m_stats.numSamples = n;
    m_stats.numClusters = activeIds.size();
    m_stats.dimensions = m_dim;
    m_stats.mergeSteps = m_merges.size();
    m_stats.cutHeight = m_merges.isEmpty() ? 0.0 : m_merges.last().distance;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_stats.numClusters, m_stats.cutHeight, timer.elapsed());
}

/* ---- Labels ---- */

QVector<int> Agglomerative10::labels() const { return m_labels; }

/* ---- Dendrogram ---- */

QVector<Agglomerative10::MergeRecord> Agglomerative10::dendrogram() const
{
    return m_merges;
}

/* ---- Reset ---- */

void Agglomerative10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_labels.clear();
    m_merges.clear();
    m_dim = 0;
}
