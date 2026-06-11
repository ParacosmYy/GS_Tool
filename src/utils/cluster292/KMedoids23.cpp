/**
 * @file KMedoids23.cpp
 * @brief KMedoids23 实现
 *
 * 实现K-中心点聚类：CLARA采样与轮廓系数引导中心点交换实现大规模鲁棒PAM聚类。
 */

#include "utils/cluster292/KMedoids23.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

KMedoids23::KMedoids23(QObject *parent)
    : QObject(parent) {}

KMedoids23::~KMedoids23() = default;

/* ---- Configuration ---- */

void KMedoids23::setNumClusters(int k) { m_k = qBound(2, k, 200); }
void KMedoids23::setMaxIterations(int maxIter) { m_maxIter = qBound(5, maxIter, 1000); }
void KMedoids23::setClaraSamples(int samples) { m_claraSamples = qBound(1, samples, 50); }
void KMedoids23::setSampleSize(int size) { m_sampleSize = qBound(32, size, 10000); }

/* ---- Euclidean distance ---- */

double KMedoids23::euclideanDistance(const QVector<double>& a,
                                      const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Compute pairwise distance matrix ---- */

QVector<QVector<double>> KMedoids23::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclideanDistance(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- Assign points to nearest medoid ---- */

QVector<int> KMedoids23::assignToMedoids(
    const QVector<QVector<double>>& distMatrix,
    const QVector<int>& medoids) const
{
    int n = distMatrix.size();
    QVector<int> assignments(n, 0);
    for (int i = 0; i < n; ++i) {
        double minDist = 1e300;
        for (int m = 0; m < medoids.size(); ++m) {
            if (medoids[m] < n && distMatrix[i][medoids[m]] < minDist) {
                minDist = distMatrix[i][medoids[m]];
                assignments[i] = m;
            }
        }
    }
    return assignments;
}

/* ---- Compute total cost ---- */

double KMedoids23::computeTotalCost(const QVector<QVector<double>>& distMatrix,
                                      const QVector<int>& medoids,
                                      const QVector<int>& assignments) const
{
    double cost = 0.0;
    int n = distMatrix.size();
    for (int i = 0; i < n; ++i) {
        if (assignments[i] >= 0 && assignments[i] < medoids.size())
            cost += distMatrix[i][medoids[assignments[i]]];
    }
    return cost;
}

/* ---- Compute silhouette score ---- */

double KMedoids23::computeSilhouette(const QVector<QVector<double>>& distMatrix,
                                       const QVector<int>& assignments, int k) const
{
    int n = distMatrix.size();
    if (n <= k) return 0.0;

    // Count cluster sizes
    QVector<int> clusterSize(k, 0);
    for (int i = 0; i < n; ++i)
        if (assignments[i] >= 0 && assignments[i] < k)
            clusterSize[assignments[i]]++;

    double totalSil = 0.0;
    for (int i = 0; i < n; ++i) {
        int ci = assignments[i];
        if (ci < 0 || ci >= k || clusterSize[ci] <= 1) continue;

        // Average distance to own cluster
        double a = 0.0;
        int countA = 0;
        for (int j = 0; j < n; ++j) {
            if (j != i && assignments[j] == ci) {
                a += distMatrix[i][j];
                countA++;
            }
        }
        a = countA > 0 ? a / countA : 0.0;

        // Minimum average distance to other clusters
        double minB = 1e300;
        for (int c = 0; c < k; ++c) {
            if (c == ci || clusterSize[c] == 0) continue;
            double b = 0.0;
            int countB = 0;
            for (int j = 0; j < n; ++j) {
                if (assignments[j] == c) {
                    b += distMatrix[i][j];
                    countB++;
                }
            }
            if (countB > 0) minB = qMin(minB, b / countB);
        }
        if (minB > 9e299) minB = 0.0;
        double denom = qMax(a, minB);
        totalSil += denom > 0 ? (minB - a) / denom : 0.0;
    }
    return totalSil / n;
}

/* ---- PAM core algorithm on a subset ---- */

KMedoids23::ClusterResult KMedoids23::pamCore(
    const QVector<QVector<double>>& distMatrix,
    const QVector<int>& indices) const
{
    int n = indices.size();
    int localK = qMin(m_k, n);
    ClusterResult result;

    // Initialize: pick k medoids by BUILD phase (greedy selection)
    QVector<int> medoids;
    QVector<bool> isMedoid(n, false);

    // First medoid: minimize total distance
    double bestTotal = 1e300;
    int firstMedoid = 0;
    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < n; ++j)
            total += distMatrix[indices[i]][indices[j]];
        if (total < bestTotal) {
            bestTotal = total;
            firstMedoid = i;
        }
    }
    medoids.append(firstMedoid);
    isMedoid[firstMedoid] = true;

    // Greedily add remaining medoids
    for (int m = 1; m < localK; ++m) {
        double bestGain = -1e300;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            if (isMedoid[i]) continue;
            // Gain: sum of max(0, currentDist - dist(i,j))
            double gain = 0.0;
            for (int j = 0; j < n; ++j) {
                if (isMedoid[j]) continue;
                double curDist = 1e300;
                for (int md : medoids)
                    curDist = qMin(curDist, distMatrix[indices[j]][indices[md]]);
                double newDist = distMatrix[indices[j]][indices[i]];
                gain += qMax(0.0, curDist - newDist);
            }
            if (gain > bestGain) {
                bestGain = gain;
                bestIdx = i;
            }
        }
        medoids.append(bestIdx);
        isMedoid[bestIdx] = true;
    }

    // Map local medoid indices to global
    QVector<int> globalMedoids;
    for (int m : medoids)
        globalMedoids.append(indices[m]);
    result.medoidIndices = globalMedoids;

    // SWAP phase: silhouette-guided medoid swap
    auto assignments = assignToMedoids(distMatrix, globalMedoids);
    double currentCost = computeTotalCost(distMatrix, globalMedoids, assignments);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool improved = false;
        for (int m = 0; m < medoids.size(); ++m) {
            double bestSwapCost = currentCost;
            int bestSwap = medoids[m];
            for (int i = 0; i < n; ++i) {
                if (isMedoid[i]) continue;
                // Try swapping medoid m with non-medoid i
                QVector<int> trialMedoids = globalMedoids;
                trialMedoids[m] = indices[i];
                auto trialAssign = assignToMedoids(distMatrix, trialMedoids);
                double trialCost = computeTotalCost(distMatrix, trialMedoids, trialAssign);
                if (trialCost < bestSwapCost) {
                    bestSwapCost = trialCost;
                    bestSwap = i;
                }
            }
            if (bestSwap != medoids[m]) {
                isMedoid[medoids[m]] = false;
                medoids[m] = bestSwap;
                isMedoid[bestSwap] = true;
                globalMedoids[m] = indices[bestSwap];
                currentCost = bestSwapCost;
                improved = true;
            }
        }
        if (!improved) {
            result.numIterations = iter + 1;
            break;
        }
        result.numIterations = iter + 1;
    }

    assignments = assignToMedoids(distMatrix, globalMedoids);
    result.assignments = assignments;
    result.totalCost = currentCost;
    result.silhouetteScore = computeSilhouette(distMatrix, assignments, localK);

    // Distance to own medoid
    result.distances.resize(distMatrix.size());
    for (int i = 0; i < distMatrix.size(); ++i) {
        if (assignments[i] >= 0 && assignments[i] < globalMedoids.size())
            result.distances[i] = distMatrix[i][globalMedoids[assignments[i]]];
        else
            result.distances[i] = 0.0;
    }

    return result;
}

/* ---- Fit: CLARA for large data, PAM for small ---- */

KMedoids23::ClusterResult KMedoids23::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    ClusterResult bestResult;

    auto distMatrix = computeDistanceMatrix(data);

    if (n <= m_sampleSize) {
        // Full PAM
        QVector<int> allIndices(n);
        for (int i = 0; i < n; ++i) allIndices[i] = i;
        bestResult = pamCore(distMatrix, allIndices);
    } else {
        // CLARA: multiple samples, keep best by silhouette
        double bestSil = -2.0;
        for (int s = 0; s < m_claraSamples; ++s) {
            // Random sample
            QVector<int> pool(n);
            for (int i = 0; i < n; ++i) pool[i] = i;
            // Fisher-Yates shuffle for sampleSize elements
            int ssize = qMin(m_sampleSize, n);
            for (int i = 0; i < ssize; ++i) {
                int j = i + (qrand() % (n - i));
                std::swap(pool[i], pool[j]);
            }
            QVector<int> sample(pool.begin(), pool.begin() + ssize);

            auto result = pamCore(distMatrix, sample);
            if (result.silhouetteScore > bestSil) {
                bestSil = result.silhouetteScore;
                // Re-assign all points to sampled medoids
                result.assignments = assignToMedoids(distMatrix, result.medoidIndices);
                result.totalCost = computeTotalCost(distMatrix, result.medoidIndices,
                                                     result.assignments);
                result.silhouetteScore = computeSilhouette(distMatrix,
                    result.assignments, m_k);
                bestResult = result;
            }
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numClusters = m_k;
    m_stats.numPoints = n;
    m_stats.totalFits++;
    m_silSum += bestResult.silhouetteScore;
    m_stats.avgSilhouette = m_silSum / m_stats.totalFits;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(n, m_k, bestResult.silhouetteScore, elapsed);
    return bestResult;
}

/* ---- Predict ---- */

QVector<int> KMedoids23::predict(const QVector<QVector<double>>& data,
                                   const QVector<QVector<double>>& medoids) const
{
    int n = data.size();
    QVector<int> result(n, 0);
    for (int i = 0; i < n; ++i) {
        double minDist = 1e300;
        for (int m = 0; m < medoids.size(); ++m) {
            double d = euclideanDistance(data[i], medoids[m]);
            if (d < minDist) {
                minDist = d;
                result[i] = m;
            }
        }
    }
    return result;
}

/* ---- Reset ---- */

void KMedoids23::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_silSum = 0.0;
}
