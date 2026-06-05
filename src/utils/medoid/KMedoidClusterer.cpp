/**
 * @file KMedoidClusterer.cpp
 * @brief K-Medoid聚类器实现
 */

#include "utils/medoid/KMedoidClusterer.h"
#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <algorithm>
#include <random>

KMedoidClusterer::KMedoidClusterer(QObject* parent)
    : QObject(parent), m_k(3), m_maxIterations(100), m_timeSum(0.0) {}

void KMedoidClusterer::setK(int k) { m_k = qMax(1, k); }
void KMedoidClusterer::setMaxIterations(int maxIter) { m_maxIterations = qMax(1, maxIter); }

QList<KMedoidClusterer::Cluster> KMedoidClusterer::cluster(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Cluster> result;
    if (data.isEmpty()) return result;

    int n = data.size();
    int k = qMin(m_k, n);

    /* 随机初始化medoid */
    std::mt19937 rng(42);
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    /* Fisher-Yates洗牌: 避免std::shuffle与QVector迭代器的兼容性问题 */
    for (int i = n - 1; i > 0; --i) {
        int j = static_cast<int>(rng() % static_cast<unsigned>(i + 1));
        std::swap(indices[i], indices[j]);
    }

    QVector<int> medoids;
    for (int i = 0; i < k; ++i) medoids.append(indices[i]);

    QVector<int> assignments(n, 0);
    double bestCost = totalCost(data, medoids, assignments);
    int totalSwaps = 0;

    /* PAM迭代 */
    for (int iter = 0; iter < m_maxIterations; ++iter) {
        bool improved = false;

        for (int m = 0; m < k; ++m) {
            int bestReplacement = medoids[m];
            double bestSwapCost = bestCost;

            for (int candidate = 0; candidate < n; ++candidate) {
                if (medoids.contains(candidate)) continue;

                QVector<int> newMedoids = medoids;
                newMedoids[m] = candidate;

                QVector<int> newAssign(n, 0);
                double newCost = totalCost(data, newMedoids, newAssign);

                if (newCost < bestSwapCost) {
                    bestSwapCost = newCost;
                    bestReplacement = candidate;
                }
            }

            if (bestReplacement != medoids[m]) {
                medoids[m] = bestReplacement;
                bestCost = bestSwapCost;
                improved = true;
                ++totalSwaps;
            }
        }

        if (!improved) break;
    }

    /* 构建结果 */
    bestCost = totalCost(data, medoids, assignments);
    for (int m = 0; m < k; ++m) {
        Cluster cl;
        cl.medoidIndex = medoids[m];
        cl.totalDistance = 0.0;
        for (int i = 0; i < n; ++i) {
            if (assignments[i] == m) {
                cl.memberIndices.append(i);
                cl.totalDistance += qAbs(data[i] - data[medoids[m]]);
            }
        }
        result.append(cl);
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalClusterings;
    m_stats.totalSwaps += totalSwaps;
    double iterSum = m_stats.avgIterations * (m_stats.totalClusterings - 1) + m_maxIterations;
    m_stats.avgIterations = iterSum / m_stats.totalClusterings;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(k, totalSwaps);
    return result;
}

double KMedoidClusterer::totalCost(const QVector<double>& data,
                                    const QVector<int>& medoids,
                                    QVector<int>& assignments) const
{
    double cost = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double minDist = std::numeric_limits<double>::max();
        int bestM = 0;
        for (int m = 0; m < medoids.size(); ++m) {
            double d = qAbs(data[i] - data[medoids[m]]);
            if (d < minDist) { minDist = d; bestM = m; }
        }
        assignments[i] = bestM;
        cost += minDist;
    }
    return cost;
}

void KMedoidClusterer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
