/**
 * @file KMedoids15.cpp
 * @brief KMedoids15 实现
 *
 * 实现K-Medoids聚类：PAM交换启发式、CLARA大规模采样、轮廓系数评估。
 */

#include "utils/cluster181/KMedoids15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

KMedoids15::KMedoids15(QObject *parent) : QObject(parent) {}
KMedoids15::~KMedoids15() = default;

/* ---- Configuration ---- */

void KMedoids15::setNumClusters(int k) { m_numClusters = qMax(1, k); }
void KMedoids15::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void KMedoids15::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }
void KMedoids15::setMetric(Metric m) { m_metric = m; }
void KMedoids15::setUseCLARA(bool enabled) { m_useCLARA = enabled; }
void KMedoids15::setClarSamples(int samples) { m_clarSamples = qMax(1, samples); }
void KMedoids15::setClarSampleSize(int size) { m_clarSampleSize = qMax(10, size); }

/* ---- Distance ---- */

double KMedoids15::distance(const QVector<double>& a, const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    if (d == 0) return 0.0;

    if (m_metric == Metric::Euclidean) {
        double sum = 0.0;
        for (int i = 0; i < d; ++i) {
            double diff = a[i] - b[i];
            sum += diff * diff;
        }
        return qSqrt(sum);
    } else if (m_metric == Metric::Manhattan) {
        double sum = 0.0;
        for (int i = 0; i < d; ++i) sum += qAbs(a[i] - b[i]);
        return sum;
    } else { // Cosine
        double dot = 0.0, na = 0.0, nb = 0.0;
        for (int i = 0; i < d; ++i) {
            dot += a[i] * b[i];
            na += a[i] * a[i];
            nb += b[i] * b[i];
        }
        double denom = qSqrt(na) * qSqrt(nb);
        return (denom < 1e-12) ? 1.0 : 1.0 - dot / denom;
    }
}

/* ---- Dissimilarity matrix ---- */

QVector<QVector<double>> KMedoids15::dissimilarityMatrix(
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

/* ---- PAM build phase ---- */

QVector<int> KMedoids15::pamBuild(const QVector<QVector<double>>& dist,
                                    int n, int k) const
{
    // Select first medoid: point with minimum total dissimilarity
    QVector<double> totalDist(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            totalDist[i] += dist[i][j];

    int first = 0;
    for (int i = 1; i < n; ++i)
        if (totalDist[i] < totalDist[first]) first = i;

    QVector<int> medoids = {first};
    QVector<double> closest(n, 0.0);
    for (int i = 0; i < n; ++i) closest[i] = dist[i][first];

    // Greedy select remaining medoids
    for (int c = 1; c < k; ++c) {
        int best = -1;
        double bestGain = -1e18;
        for (int j = 0; j < n; ++j) {
            if (medoids.contains(j)) continue;
            double gain = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = closest[i] - dist[i][j];
                if (diff > 0) gain += diff;
            }
            if (gain > bestGain) { bestGain = gain; best = j; }
        }
        if (best < 0) best = (medoids.last() + 1) % n;
        medoids.append(best);
        for (int i = 0; i < n; ++i)
            closest[i] = qMin(closest[i], dist[i][best]);
    }
    return medoids;
}

/* ---- PAM swap phase ---- */

double KMedoids15::pamSwap(const QVector<QVector<double>>& dist,
                             QVector<int>& medoids, int n, int k)
{
    int iter = 0;
    double prevCost = 1e18;

    while (iter++ < m_maxIterations) {
        QVector<int> labels = assign(dist, medoids, n);

        // Compute current total cost
        double totalCost = 0.0;
        for (int i = 0; i < n; ++i)
            totalCost += dist[i][medoids[labels[i]]];

        if (qAbs(prevCost - totalCost) < m_tolerance) break;
        prevCost = totalCost;

        // Try swapping each medoid with each non-medoid
        bool improved = false;
        for (int m = 0; m < k; ++m) {
            double bestDelta = 0.0;
            int bestSwap = -1;

            for (int j = 0; j < n; ++j) {
                if (medoids.contains(j)) continue;

                // Compute cost change for swapping medoids[m] with j
                double delta = 0.0;
                for (int i = 0; i < n; ++i) {
                    double curDist = dist[i][medoids[m]];
                    // Find closest medoid excluding m
                    double nextBest = 1e18;
                    for (int mm = 0; mm < k; ++mm) {
                        if (mm == m) continue;
                        nextBest = qMin(nextBest, dist[i][medoids[mm]]);
                    }
                    double newDist = qMin(dist[i][j], nextBest);
                    delta += newDist - qMin(curDist, nextBest);
                }
                if (delta < bestDelta) { bestDelta = delta; bestSwap = j; }
            }

            if (bestSwap >= 0) {
                medoids[m] = bestSwap;
                improved = true;
            }
        }
        if (!improved) break;
    }

    // Final cost
    double totalCost = 0.0;
    QVector<int> labels = assign(dist, medoids, n);
    for (int i = 0; i < n; ++i) totalCost += dist[i][medoids[labels[i]]];
    return totalCost;
}

/* ---- Assign ---- */

QVector<int> KMedoids15::assign(const QVector<QVector<double>>& dist,
                                  const QVector<int>& medoids, int n) const
{
    int k = medoids.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double minD = 1e18;
        for (int c = 0; c < k; ++c) {
            if (dist[i][medoids[c]] < minD) {
                minD = dist[i][medoids[c]];
                labels[i] = c;
            }
        }
    }
    return labels;
}

/* ---- CLARA ---- */

QVector<int> KMedoids15::sampleIndices(int n, int size) const
{
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    int s = qMin(size, n);
    for (int i = 0; i < s; ++i) {
        int j = i + qrand() % (n - i);
        std::swap(indices[i], indices[j]);
    }
    return indices.mid(0, s);
}

QVector<int> KMedoids15::mapLabels(const QVector<QVector<double>>& data,
                                      const QVector<int>& sampledIdx,
                                      const QVector<QVector<double>>& sampledData,
                                      const QVector<int>& localLabels) const
{
    int n = data.size();
    QVector<int> fullLabels(n, 0);
    // Assign each full point to nearest medoid found in sample
    QVector<QVector<double>> sampledDist = dissimilarityMatrix(sampledData);
    int k = *std::max_element(localLabels.begin(), localLabels.end()) + 1;
    QVector<int> localMedoids(k);
    for (int c = 0; c < k; ++c) {
        for (int i = 0; i < localLabels.size(); ++i) {
            if (localLabels[i] == c) { localMedoids[c] = sampledIdx[i]; break; }
        }
    }
    for (int i = 0; i < n; ++i) {
        double minD = 1e18;
        for (int c = 0; c < k; ++c) {
            double d = distance(data[i], data[localMedoids[c]]);
            if (d < minD) { minD = d; fullLabels[i] = c; }
        }
    }
    return fullLabels;
}

QVector<int> KMedoids15::claraFit(const QVector<QVector<double>>& data)
{
    int n = data.size();
    int k = m_numClusters;
    QVector<int> bestLabels;
    double bestCost = 1e18;

    for (int s = 0; s < m_clarSamples; ++s) {
        int ss = qMin(m_clarSampleSize, n);
        QVector<int> idx = sampleIndices(n, ss);
        QVector<QVector<double>> sampled(ss);
        for (int i = 0; i < ss; ++i) sampled[i] = data[idx[i]];

        auto sDist = dissimilarityMatrix(sampled);
        auto meds = pamBuild(sDist, ss, k);
        pamSwap(sDist, meds, ss, k);
        QVector<int> localLabels = assign(sDist, meds, ss);

        QVector<int> fullLabels = mapLabels(data, idx, sampled, localLabels);

        // Compute full cost
        double cost = 0.0;
        QVector<int> medIdx(k);
        for (int c = 0; c < k; ++c) {
            for (int i = 0; i < ss; ++i) {
                if (localLabels[i] == c) { medIdx[c] = idx[i]; break; }
            }
        }
        for (int i = 0; i < n; ++i) {
            double minD = 1e18;
            for (int c = 0; c < k; ++c)
                minD = qMin(minD, distance(data[i], data[medIdx[c]]));
            cost += minD;
        }
        if (cost < bestCost) { bestCost = cost; bestLabels = fullLabels; }
    }
    return bestLabels;
}

/* ---- Silhouette ---- */

double KMedoids15::silhouette(const QVector<QVector<double>>& data,
                                const QVector<int>& labels) const
{
    int n = data.size();
    if (n == 0) return 0.0;
    int k = *std::max_element(labels.begin(), labels.end()) + 1;
    if (k <= 1) return 0.0;

    double total = 0.0;
    for (int i = 0; i < n; ++i) {
        QVector<double> intra(k, 0.0);
        QVector<int> count(k, 0);
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            intra[labels[j]] += distance(data[i], data[j]);
            count[labels[j]]++;
        }

        double a = (count[labels[i]] > 0) ? intra[labels[i]] / count[labels[i]] : 0.0;
        double b = 1e18;
        for (int c = 0; c < k; ++c) {
            if (c == labels[i] || count[c] == 0) continue;
            b = qMin(b, intra[c] / count[c]);
        }
        if (b > 1e17) b = 0.0;
        double denom = qMax(a, b);
        total += (denom > 0) ? (b - a) / denom : 0.0;
    }
    return total / n;
}

/* ---- Main fit ---- */

QVector<int> KMedoids15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    int k = qMin(m_numClusters, n);

    if (m_useCLARA && n > m_clarSampleSize) {
        m_labels = claraFit(data);
    } else {
        auto dist = dissimilarityMatrix(data);
        m_medoids = pamBuild(dist, n, k);
        pamSwap(dist, m_medoids, n, k);
        m_labels = assign(dist, m_medoids, n);
    }

    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        if (m_medoids.size() > m_labels[i])
            cost += distance(data[i], data[m_medoids[m_labels[i]]]);
    }
    double sil = silhouette(data, m_labels);

    m_stats.totalRuns++;
    m_stats.numClusters = k;
    m_stats.totalCost = cost;
    m_stats.silhouetteScore = sil;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(k, cost, sil);
    return m_labels;
}

/* ---- Accessors ---- */

QVector<int> KMedoids15::medoidIndices() const { return m_medoids; }

/* ---- Reset ---- */

void KMedoids15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_medoids.clear();
    m_labels.clear();
}
