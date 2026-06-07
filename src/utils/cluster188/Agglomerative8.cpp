/**
 * @file Agglomerative8.cpp
 * @brief Agglomerative8 实现
 *
 * 实现层次凝聚聚类：全连接距离、Genolini共识合并、树状图输出。
 */

#include "utils/cluster188/Agglomerative8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative8::Agglomerative8(QObject *parent) : QObject(parent) {}
Agglomerative8::~Agglomerative8() = default;

/* ---- Configuration ---- */

void Agglomerative8::setTargetClusters(int k) { m_targetClusters = qMax(1, k); }
void Agglomerative8::setDistanceThreshold(double t) { m_distThreshold = qMax(0.0, t); }
void Agglomerative8::setConsensusAlpha(double a) { m_consensusAlpha = qBound(0.0, a, 1.0); }
void Agglomerative8::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }

/* ---- Pairwise distance matrix ---- */

QVector<QVector<double>> Agglomerative8::computeDistMatrix(
    const QVector<QVector<double>>& data) const
{
    int N = data.size();
    QVector<QVector<double>> dist(N, QVector<double>(N, 0.0));
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            double sum = 0.0;
            for (int d = 0; d < data[i].size(); ++d) {
                double diff = data[i][d] - data[j][d];
                sum += diff * diff;
            }
            dist[i][j] = dist[j][i] = qSqrt(sum);
        }
    }
    return dist;
}

/* ---- Complete-linkage distance ---- */

double Agglomerative8::completeLinkage(const QVector<int>& c1, const QVector<int>& c2,
                                        const QVector<QVector<double>>& dist) const
{
    double maxDist = 0.0;
    for (int i : c1) {
        for (int j : c2) {
            if (dist[i][j] > maxDist)
                maxDist = dist[i][j];
        }
    }
    return maxDist;
}

/* ---- Average intra-cluster distance ---- */

double Agglomerative8::avgIntraDistance(const QVector<int>& cluster,
                                         const QVector<QVector<double>>& dist) const
{
    if (cluster.size() < 2) return 0.0;
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < cluster.size(); ++i) {
        for (int j = i + 1; j < cluster.size(); ++j) {
            sum += dist[cluster[i]][cluster[j]];
            count++;
        }
    }
    return (count > 0) ? sum / count : 0.0;
}

/* ---- Genolini consensus criterion ---- */

bool Agglomerative8::consensusCriterion(double linkDist, double avgIntra1,
                                         double avgIntra2) const
{
    // Weighted consensus: merge if link distance is within alpha * max(intra)
    double threshold = m_consensusAlpha * qMax(avgIntra1, avgIntra2);
    return linkDist <= qMax(threshold, 1e-10);
}

/* ---- Main fit ---- */

QVector<int> Agglomerative8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    if (N == 0) return {};

    // Compute distance matrix
    auto dist = computeDistMatrix(data);

    // Initialize: each point is its own cluster
    QVector<QVector<int>> clusters(N);
    for (int i = 0; i < N; ++i)
        clusters[i].append(i);

    m_merges.clear();
    int mergeStep = 0;

    while (clusters.size() > m_targetClusters && mergeStep < m_maxIterations) {
        // Find closest pair using complete linkage
        double bestDist = std::numeric_limits<double>::max();
        int bestI = 0, bestJ = 1;

        for (int i = 0; i < clusters.size(); ++i) {
            for (int j = i + 1; j < clusters.size(); ++j) {
                double d = completeLinkage(clusters[i], clusters[j], dist);
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        // Check distance threshold
        if (m_distThreshold > 0.0 && bestDist > m_distThreshold)
            break;

        // Genolini consensus check
        double intra1 = avgIntraDistance(clusters[bestI], dist);
        double intra2 = avgIntraDistance(clusters[bestJ], dist);
        if (!consensusCriterion(bestDist, intra1, intra2))
            break;

        // Record merge
        MergeRecord rec;
        rec.clusterA = bestI;
        rec.clusterB = bestJ;
        rec.distance = bestDist;
        rec.newSize = clusters[bestI].size() + clusters[bestJ].size();
        m_merges.append(rec);

        // Merge clusters
        clusters[bestI].append(clusters[bestJ]);
        clusters.removeAt(bestJ);
        mergeStep++;
    }

    // Assign labels
    QVector<int> labels(N);
    for (int c = 0; c < clusters.size(); ++c)
        for (int idx : clusters[c])
            labels[idx] = c;

    // Compute cluster centers
    int D = data[0].size();
    m_centers.resize(clusters.size());
    for (int c = 0; c < clusters.size(); ++c) {
        m_centers[c].resize(D, 0.0);
        for (int idx : clusters[c])
            for (int d = 0; d < D; ++d)
                m_centers[c][d] += data[idx][d];
        for (int d = 0; d < D; ++d)
            m_centers[c][d] /= clusters[c].size();
    }

    m_stats.totalRuns++;
    m_stats.numPoints = N;
    m_stats.numClusters = clusters.size();
    m_stats.mergeSteps = mergeStep;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(clusters.size(), mergeStep, timer.elapsed());
    return labels;
}

/* ---- Reset ---- */

void Agglomerative8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_merges.clear();
    m_centers.clear();
}
