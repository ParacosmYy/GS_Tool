/**
 * @file KMedoids14.cpp
 * @brief KMedoids14 实现
 *
 * 实现K-Medoids/PAM聚类：CLARA采样、轮廓系数验证。
 */

#include "utils/cluster172/KMedoids14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

KMedoids14::KMedoids14(QObject *parent)
    : QObject(parent)
{
}

KMedoids14::~KMedoids14() = default;

/* ---- Configuration ---- */

void KMedoids14::setK(int k) { m_k = qMax(1, k); }
void KMedoids14::setDistanceMetric(DistanceMetric m) { m_metric = m; }
void KMedoids14::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void KMedoids14::setClaraSamples(int s) { m_claraSamples = qMax(1, s); }
void KMedoids14::setClaraSampleRatio(double r) { m_claraRatio = qBound(0.01, r, 1.0); }

/* ---- Distance computation ---- */

double KMedoids14::distance(const QVector<double>& a,
                             const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    double sum = 0.0;
    if (m_metric == Euclidean) {
        for (int i = 0; i < d; ++i)
            sum += qPow(a[i] - b[i], 2);
        return qSqrt(sum);
    }
    /* Manhattan */
    for (int i = 0; i < d; ++i)
        sum += qAbs(a[i] - b[i]);
    return sum;
}

/* ---- PAM BUILD phase ---- */

void KMedoids14::pamBuild(const QVector<QVector<double>>& data,
                           const QVector<int>& indices)
{
    int n = indices.size();
    m_medoidIdx.clear();
    m_medoidIdx.reserve(m_k);

    /* Select first medoid: minimize total distance */
    double bestTotal = 1e30;
    int first = 0;
    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < n; ++j)
            total += distance(data[indices[i]], data[indices[j]]);
        if (total < bestTotal) { bestTotal = total; first = i; }
    }
    m_medoidIdx.append(indices[first]);

    /* Greedy selection of remaining medoids */
    QVector<double> nearestDist(n, 1e30);
    for (int k = 1; k < m_k; ++k) {
        /* Update nearest distances */
        int lastMed = m_medoidIdx.last();
        int lastPos = 0;
        for (int i = 0; i < n; ++i) {
            if (indices[i] == lastMed) lastPos = i;
            double d = distance(data[indices[i]], data[lastMed]);
            nearestDist[i] = qMin(nearestDist[i], d);
        }
        /* Select point that maximizes reduction */
        double bestGain = -1e30;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            double gain = 0.0;
            for (int j = 0; j < n; ++j) {
                double d = distance(data[indices[i]], data[indices[j]]);
                gain += qMax(0.0, nearestDist[j] - d);
            }
            if (gain > bestGain) { bestGain = gain; chosen = i; }
        }
        m_medoidIdx.append(indices[chosen]);
    }
}

/* ---- PAM SWAP phase ---- */

double KMedoids14::pamSwap(const QVector<QVector<double>>& data,
                             const QVector<int>& indices)
{
    int n = indices.size();
    int k = m_medoidIdx.size();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Assign each point to nearest medoid */
        QVector<int> assign(n, 0);
        double totalCost = 0.0;
        for (int i = 0; i < n; ++i) {
            double bestD = 1e30;
            for (int m = 0; m < k; ++m) {
                double d = distance(data[indices[i]], data[m_medoidIdx[m]]);
                if (d < bestD) { bestD = d; assign[i] = m; }
            }
            totalCost += bestD;
        }

        /* Try swapping each medoid with each non-medoid */
        bool improved = false;
        for (int m = 0; m < k; ++m) {
            double bestSwapCost = totalCost;
            int bestSwap = -1;
            for (int i = 0; i < n; ++i) {
                if (indices[i] == m_medoidIdx[m]) continue;
                /* Compute swap cost */
                double swapCost = 0.0;
                int candidate = indices[i];
                for (int j = 0; j < n; ++j) {
                    double origD = 1e30;
                    for (int mm = 0; mm < k; ++mm) {
                        if (mm == m) continue;
                        origD = qMin(origD,
                            distance(data[indices[j]], data[m_medoidIdx[mm]]));
                    }
                    double newD = distance(data[indices[j]], data[candidate]);
                    swapCost += qMin(origD, newD);
                }
                if (swapCost < bestSwapCost) {
                    bestSwapCost = swapCost;
                    bestSwap = i;
                }
            }
            if (bestSwap >= 0 && bestSwapCost < totalCost) {
                m_medoidIdx[m] = indices[bestSwap];
                improved = true;
                break; /* Restart from assignment */
            }
        }
        if (!improved) break;
    }

    /* Final cost */
    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        double bestD = 1e30;
        for (int m = 0; m < k; ++m)
            bestD = qMin(bestD, distance(data[indices[i]], data[m_medoidIdx[m]]));
        cost += bestD;
    }
    return cost;
}

/* ---- Assign clusters ---- */

QVector<int> KMedoids14::assignClusters(const QVector<QVector<double>>& data,
                                         const QVector<int>& indices) const
{
    int n = indices.size();
    int k = m_medoidIdx.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestD = 1e30;
        for (int m = 0; m < k; ++m) {
            double d = distance(data[indices[i]], data[m_medoidIdx[m]]);
            if (d < bestD) { bestD = d; labels[i] = m; }
        }
    }
    return labels;
}

/* ---- Main fit (PAM on full dataset) ---- */

QVector<int> KMedoids14::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    pamBuild(data, indices);
    double cost = pamSwap(data, indices);
    m_labels = assignClusters(data, indices);

    /* Map labels back to 0..n-1 */
    QVector<int> fullLabels(n);
    for (int i = 0; i < n; ++i) fullLabels[i] = m_labels[i];

    double sil = silhouetteScore(data, fullLabels);

    m_stats.totalRuns++;
    m_stats.lastK = m_k;
    m_stats.lastCost = cost;
    m_stats.lastSilhouette = sil;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_k, cost, sil);
    return fullLabels;
}

/* ---- CLARA sampling ---- */

QVector<int> KMedoids14::fitClara(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    int sampleSize = qMax(m_k + 1, static_cast<int>(n * m_claraRatio));
    sampleSize = qMin(sampleSize, n);

    double bestCost = 1e30;
    QVector<int> bestMedoids;
    QVector<int> bestLabels(n, 0);

    for (int s = 0; s < m_claraSamples; ++s) {
        /* Random sample (Fisher-Yates partial shuffle) */
        QVector<int> pool(n);
        for (int i = 0; i < n; ++i) pool[i] = i;
        for (int i = 0; i < sampleSize; ++i) {
            int j = i + (qrand() % (n - i));
            int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
        }
        QVector<int> sampleIdx = pool.mid(0, sampleSize);

        pamBuild(data, sampleIdx);
        double cost = pamSwap(data, sampleIdx);
        QVector<int> sampleLabels = assignClusters(data, sampleIdx);

        if (cost < bestCost) {
            bestCost = cost;
            bestMedoids = m_medoidIdx;
        }

        emit claraIterationCompleted(s, bestCost);
    }

    /* Assign all points using best medoids */
    m_medoidIdx = bestMedoids;
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        double bestD = 1e30;
        for (int m = 0; m < m_medoidIdx.size(); ++m) {
            double d = distance(data[i], data[m_medoidIdx[m]]);
            if (d < bestD) { bestD = d; m_labels[i] = m; }
        }
    }

    double sil = silhouetteScore(data, m_labels);
    m_stats.totalRuns++;
    m_stats.lastK = m_k;
    m_stats.lastCost = bestCost;
    m_stats.lastSilhouette = sil;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_k, bestCost, sil);
    return m_labels;
}

/* ---- Single silhouette ---- */

double KMedoids14::singleSilhouette(const QVector<QVector<double>>& data,
                                      const QVector<int>& labels, int i) const
{
    int n = data.size();
    int myCluster = labels[i];

    /* Compute a(i): avg distance to same cluster */
    double aSum = 0.0;
    int aCount = 0;
    for (int j = 0; j < n; ++j) {
        if (j == i || labels[j] != myCluster) continue;
        aSum += distance(data[i], data[j]);
        aCount++;
    }
    double a = (aCount > 0) ? aSum / aCount : 0.0;

    /* Compute b(i): min avg distance to other clusters */
    int maxLabel = *std::max_element(labels.begin(), labels.end());
    double b = 1e30;
    for (int c = 0; c <= maxLabel; ++c) {
        if (c == myCluster) continue;
        double cSum = 0.0;
        int cCount = 0;
        for (int j = 0; j < n; ++j) {
            if (labels[j] != c) continue;
            cSum += distance(data[i], data[j]);
            cCount++;
        }
        if (cCount > 0) b = qMin(b, cSum / cCount);
    }
    if (b > 1e29) return 0.0;

    double denom = qMax(a, b);
    return (denom > 0.0) ? (b - a) / denom : 0.0;
}

/* ---- Silhouette score ---- */

double KMedoids14::silhouetteScore(const QVector<QVector<double>>& data,
                                    const QVector<int>& labels) const
{
    int n = data.size();
    if (n == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += singleSilhouette(data, labels, i);
    return sum / n;
}

/* ---- Accessors ---- */

QVector<int> KMedoids14::medoidIndices() const { return m_medoidIdx; }

void KMedoids14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
