/**
 * @file Agglomerative11.cpp
 * @brief Agglomerative11 实现
 *
 * 实现层次凝聚聚类：WPGMA加权配对平均与轮廓系数最优切割高度。
 */

#include "utils/cluster231/Agglomerative11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative11::Agglomerative11(QObject *parent) : QObject(parent) {}
Agglomerative11::~Agglomerative11() = default;

/* ---- Configuration ---- */

void Agglomerative11::setLinkage(int mode)
{
    m_linkage = qBound(0, mode, 0);  // WPGMA only for now
}

/* ---- Squared Euclidean distance ---- */

double Agglomerative11::squaredDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

/* ---- Pairwise distance matrix ---- */

QVector<double> Agglomerative11::computeDistanceMatrix(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<double> dm(n * n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = qSqrt(squaredDist(data[i], data[j]));
            dm[i * n + j] = d;
            dm[j * n + i] = d;
        }
    return dm;
}

/* ---- WPGMA distance between two clusters ---- */

double Agglomerative11::wpgmaDistance(int ci, int cj, const QVector<double>& distMatrix,
                                       int n, const QVector<int>& sizes) const
{
    // WPGMA: average of all pairwise distances weighted by cluster sizes
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (m_clusterMap[i] != ci) continue;
        for (int j = 0; j < n; ++j) {
            if (m_clusterMap[j] != cj) continue;
            sum += distMatrix[i * n + j];
            count++;
        }
    }
    return (count > 0) ? sum / count : 0.0;
}

/* ---- Silhouette index ---- */

double Agglomerative11::computeSilhouette(const QVector<int>& labels) const
{
    int n = m_data.size();
    if (n == 0) return 0.0;

    // Count unique labels
    int maxLabel = *std::max_element(labels.begin(), labels.end());
    int K = maxLabel + 1;
    if (K <= 1) return 0.0;

    double totalSil = 0.0;
    for (int i = 0; i < n; ++i) {
        int ci = labels[i];

        // Compute average intra-cluster distance a(i)
        double aSum = 0.0;
        int aCount = 0;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            if (labels[j] == ci) {
                aSum += qSqrt(squaredDist(m_data[i], m_data[j]));
                aCount++;
            }
        }
        double a = (aCount > 0) ? aSum / aCount : 0.0;

        // Compute minimum average inter-cluster distance b(i)
        double bMin = std::numeric_limits<double>::max();
        for (int k = 0; k < K; ++k) {
            if (k == ci) continue;
            double bSum = 0.0;
            int bCount = 0;
            for (int j = 0; j < n; ++j) {
                if (labels[j] == k) {
                    bSum += qSqrt(squaredDist(m_data[i], m_data[j]));
                    bCount++;
                }
            }
            if (bCount > 0) bMin = qMin(bMin, bSum / bCount);
        }
        if (bMin == std::numeric_limits<double>::max()) bMin = 0.0;

        double denom = qMax(a, bMin);
        totalSil += (denom > 0.0) ? (bMin - a) / denom : 0.0;
    }
    return totalSil / n;
}

/* ---- Find optimal cut via silhouette maximization ---- */

int Agglomerative11::findOptimalCut() const
{
    int n = m_data.size();
    int bestK = 2;
    double bestSil = -2.0;
    int maxK = qMin(n, qMax(2, static_cast<int>(m_merges.size())));

    for (int k = 2; k <= maxK; ++k) {
        QVector<int> labels = cutAtK(k);
        double sil = computeSilhouette(labels);
        if (sil > bestSil) {
            bestSil = sil;
            bestK = k;
        }
    }
    return bestK;
}

/* ---- Fit ---- */

bool Agglomerative11::fit(const QVector<QVector<double>>& data, int maxClusters)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return false;
    m_data = data;
    int d = data[0].size();

    // Initialize: each point is its own cluster
    m_clusterMap.resize(n);
    for (int i = 0; i < n; ++i) m_clusterMap[i] = i;

    QVector<int> sizes(n, 1);
    QVector<double> distMatrix = computeDistanceMatrix(data);
    m_merges.clear();

    // Active cluster set
    QVector<bool> active(n, true);

    int nextId = n;
    // Expand cluster map for merged clusters
    // We'll use a flat distance cache approach
    QVector<double> mergeDist(n * n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            mergeDist[i * n + j] = distMatrix[i * n + j];

    for (int step = 0; step < n - 1; ++step) {
        // Find closest pair among active clusters
        double minDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (mergeDist[i * n + j] < minDist) {
                    minDist = mergeDist[i * n + j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        // Record merge step
        MergeStep ms;
        ms.clusterA = bestI;
        ms.clusterB = bestJ;
        ms.distance = minDist;
        ms.newSize = sizes[bestI] + sizes[bestJ];
        m_merges.append(ms);

        emit mergeCompleted(step, minDist);

        // Update distances using WPGMA formula
        // d(new, k) = (size_i * d(i,k) + size_j * d(j,k)) / (size_i + size_j)
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == bestI || k == bestJ) continue;
            double dNew = (sizes[bestI] * mergeDist[bestI * n + k]
                           + sizes[bestJ] * mergeDist[bestJ * n + k])
                          / (sizes[bestI] + sizes[bestJ]);
            mergeDist[bestI * n + k] = dNew;
            mergeDist[k * n + bestI] = dNew;
        }

        // Merge bestJ into bestI
        sizes[bestI] += sizes[bestJ];
        active[bestJ] = false;

        // Remap all points in bestJ's cluster to bestI
        for (int p = 0; p < n; ++p) {
            if (m_clusterMap[p] == bestJ)
                m_clusterMap[p] = bestI;
        }
    }

    // Find optimal cut via Silhouette
    int optK = (maxClusters > 0) ? qMin(maxClusters, n) : findOptimalCut();
    QVector<int> finalLabels = cutAtK(optK);

    m_assignments.resize(n);
    for (int i = 0; i < n; ++i) {
        m_assignments[i].clusterId = finalLabels[i];
    }

    // Compute per-point silhouette
    QVector<double> sils(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int ci = finalLabels[i];
        double aSum = 0.0;
        int aCnt = 0;
        for (int j = 0; j < n; ++j) {
            if (j == i || finalLabels[j] != ci) continue;
            aSum += qSqrt(squaredDist(data[i], data[j]));
            aCnt++;
        }
        double a = (aCnt > 0) ? aSum / aCnt : 0.0;
        double bMin = std::numeric_limits<double>::max();
        for (int k = 0; k < optK; ++k) {
            if (k == ci) continue;
            double bSum = 0.0;
            int bCnt = 0;
            for (int j = 0; j < n; ++j) {
                if (finalLabels[j] == k) {
                    bSum += qSqrt(squaredDist(data[i], data[j]));
                    bCnt++;
                }
            }
            if (bCnt > 0) bMin = qMin(bMin, bSum / bCnt);
        }
        if (bMin == std::numeric_limits<double>::max()) bMin = 0.0;
        double denom = qMax(a, bMin);
        m_assignments[i].silhouette = (denom > 0) ? (bMin - a) / denom : 0.0;
    }

    double bestSil = 0.0;
    for (int i = 0; i < n; ++i) bestSil += m_assignments[i].silhouette;
    bestSil /= n;

    m_stats.numPoints = n;
    m_stats.numDimensions = d;
    m_stats.optimalClusters = optK;
    m_stats.bestSilhouette = bestSil;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(optK, bestSil, timer.elapsed());
    return true;
}

/* ---- Cut at K clusters ---- */

QVector<int> Agglomerative11::cutAtK(int k) const
{
    int n = m_data.size();
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) labels[i] = i;

    // Replay merges but stop when we have k clusters
    int mergesToApply = n - k;
    mergesToApply = qBound(0, mergesToApply, m_merges.size());

    for (int s = 0; s < mergesToApply; ++s) {
        int a = m_merges[s].clusterA;
        int b = m_merges[s].clusterB;
        for (int i = 0; i < n; ++i) {
            if (labels[i] == b) labels[i] = a;
        }
    }

    // Relabel to 0..k-1
    QMap<int, int> remap;
    int next = 0;
    for (int i = 0; i < n; ++i) {
        if (!remap.contains(labels[i]))
            remap[labels[i]] = next++;
        labels[i] = remap[labels[i]];
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<Agglomerative11::Assignment> Agglomerative11::assignments() const { return m_assignments; }
QVector<Agglomerative11::MergeStep> Agglomerative11::dendrogram() const { return m_merges; }
QVector<double> Agglomerative11::silhouetteScores() const
{
    QVector<double> s;
    s.reserve(m_assignments.size());
    for (const auto& a : m_assignments) s.append(a.silhouette);
    return s;
}

/* ---- Reset ---- */

void Agglomerative11::resetStatistics()
{
    m_data.clear();
    m_assignments.clear();
    m_merges.clear();
    m_clusterMap.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
