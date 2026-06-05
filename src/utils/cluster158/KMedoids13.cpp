/**
 * @file KMedoids13.cpp
 * @brief K-Medoids聚类算法(PAM)实现
 *
 * 实现完整的PAM算法：距离矩阵预计算、三种初始化策略、
 * BUILD+SWAP迭代优化、轮廓系数评估。
 */

#include "utils/cluster158/KMedoids13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
KMedoids13::KMedoids13(QObject* parent)
    : QObject(parent)
{
}

void KMedoids13::setK(int k)
{
    m_k = qMax(2, k);
}

void KMedoids13::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

void KMedoids13::setInitMethod(InitMethod method)
{
    m_initMethod = method;
}

/**
 * @brief 预计算全对距离矩阵
 */
QVector<QVector<double>> KMedoids13::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }
    return dist;
}

void KMedoids13::initRandom(int n)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, n - 1);
    m_medoidIndices.clear();
    QSet<int> chosen;
    while (m_medoidIndices.size() < m_k) {
        int idx = dist(rng);
        if (!chosen.contains(idx)) {
            chosen.insert(idx);
            m_medoidIndices.append(idx);
        }
    }
}

void KMedoids13::initHeuristic(const QVector<QVector<double>>& distMatrix)
{
    const int n = distMatrix.size();
    /* 选择距离和最小的K个点(最集中的点) */
    QVector<QPair<double, int>> scoreIdx;
    scoreIdx.reserve(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += distMatrix[i][j];
        }
        scoreIdx.append({sum, i});
    }
    std::sort(scoreIdx.begin(), scoreIdx.end());
    m_medoidIndices.clear();
    for (int i = 0; i < m_k && i < n; ++i) {
        m_medoidIndices.append(scoreIdx[i].second);
    }
}

void KMedoids13::initKMedoidsPP(const QVector<QVector<double>>& distMatrix)
{
    const int n = distMatrix.size();
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uniDist(0, n - 1);

    m_medoidIndices.clear();
    int first = uniDist(rng);
    m_medoidIndices.append(first);

    QVector<double> minDists(n, std::numeric_limits<double>::max());
    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        int lastMedoid = m_medoidIndices.last();
        for (int i = 0; i < n; ++i) {
            minDists[i] = qMin(minDists[i], distMatrix[i][lastMedoid]);
            totalDist += minDists[i];
        }
        std::uniform_real_distribution<double> probDist(0.0, totalDist);
        double threshold = probDist(rng);
        double cumulative = 0.0;
        for (int i = 0; i < n; ++i) {
            cumulative += minDists[i];
            if (cumulative >= threshold) {
                m_medoidIndices.append(i);
                break;
            }
        }
    }
}

/**
 * @brief 执行K-Medoids聚类(PAM算法)
 *
 * 1) 预计算距离矩阵
 * 2) 根据策略初始化medoid
 * 3) SWAP迭代：尝试用非medoid替换medoid，若总代价下降则接受
 */
QVector<int> KMedoids13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k) {
        m_stats.totalClusterOps++;
        return QVector<int>();
    }

    m_lastData = data;
    auto distMatrix = computeDistanceMatrix(data);
    const int n = data.size();

    /* 初始化 */
    switch (m_initMethod) {
    case InitMethod::Random:     initRandom(n); break;
    case InitMethod::Heuristic:  initHeuristic(distMatrix); break;
    case InitMethod::KMedoidsPP: initKMedoidsPP(distMatrix); break;
    }

    /* 初始分配 */
    auto assignLabels = [&]() -> QVector<int> {
        QVector<int> labels(n, 0);
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            for (int m = 0; m < m_medoidIndices.size(); ++m) {
                if (distMatrix[i][m_medoidIndices[m]] < bestDist) {
                    bestDist = distMatrix[i][m_medoidIndices[m]];
                    labels[i] = m;
                }
            }
        }
        return labels;
    };

    /* 计算总代价 */
    auto totalCost = [&](const QVector<int>& labels) -> double {
        double cost = 0.0;
        for (int i = 0; i < n; ++i) {
            cost += distMatrix[i][m_medoidIndices[labels[i]]];
        }
        return cost;
    };

    int iterationsUsed = 0;
    QVector<int> labels = assignLabels();
    double bestCost = totalCost(labels);

    /* PAM SWAP迭代 */
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool improved = false;

        for (int m = 0; m < m_k; ++m) {
            int currentMedoid = m_medoidIndices[m];
            double bestSwapCost = bestCost;
            int bestSwapCandidate = currentMedoid;

            for (int candidate = 0; candidate < n; ++candidate) {
                if (m_medoidIndices.contains(candidate)) continue;

                /* 试探替换 */
                m_medoidIndices[m] = candidate;
                QVector<int> trialLabels = assignLabels();
                double trialCost = totalCost(trialLabels);

                if (trialCost < bestSwapCost) {
                    bestSwapCost = trialCost;
                    bestSwapCandidate = candidate;
                }
            }

            if (bestSwapCandidate != currentMedoid) {
                m_medoidIndices[m] = bestSwapCandidate;
                bestCost = bestSwapCost;
                improved = true;
            } else {
                m_medoidIndices[m] = currentMedoid;
            }
        }

        iterationsUsed++;
        if (!improved) break;
    }

    labels = assignLabels();

    /* 提取medoid向量 */
    m_medoids.clear();
    for (int idx : m_medoidIndices) {
        m_medoids.append(data[idx]);
    }

    /* 轮廓系数 */
    m_stats.silhouetteScore = computeSilhouette(labels, distMatrix);

    m_stats.totalClusterOps++;
    m_stats.totalIterations += iterationsUsed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalClusterOps > 0)
        ? m_timeSum / m_stats.totalClusterOps : 0.0;

    emit fitCompleted(m_k, iterationsUsed, m_stats.silhouetteScore);
    return labels;
}

/**
 * @brief 预测新样本的簇归属
 */
QVector<int> KMedoids13::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels;
    labels.reserve(data.size());
    for (const auto& point : data) {
        double bestDist = std::numeric_limits<double>::max();
        int bestLabel = 0;
        for (int j = 0; j < m_medoids.size(); ++j) {
            double d = euclidean(point, m_medoids[j]);
            if (d < bestDist) {
                bestDist = d;
                bestLabel = j;
            }
        }
        labels.append(bestLabel);
    }
    return labels;
}

double KMedoids13::computeSilhouette(const QVector<int>& labels,
                                     const QVector<QVector<double>>& distMatrix) const
{
    const int n = labels.size();
    if (n <= 1) return 0.0;

    double totalSil = 0.0;
    for (int i = 0; i < n; ++i) {
        int myCluster = labels[i];

        /* 计算簇内平均距离 a(i) */
        double a = 0.0;
        int countA = 0;
        for (int j = 0; j < n; ++j) {
            if (j != i && labels[j] == myCluster) {
                a += distMatrix[i][j];
                countA++;
            }
        }
        a = (countA > 0) ? a / countA : 0.0;

        /* 计算最近其他簇的平均距离 b(i) */
        double b = std::numeric_limits<double>::max();
        for (int c = 0; c < m_k; ++c) {
            if (c == myCluster) continue;
            double avgDist = 0.0;
            int countB = 0;
            for (int j = 0; j < n; ++j) {
                if (labels[j] == c) {
                    avgDist += distMatrix[i][j];
                    countB++;
                }
            }
            if (countB > 0) {
                avgDist /= countB;
                b = qMin(b, avgDist);
            }
        }
        if (b == std::numeric_limits<double>::max()) b = 0.0;

        double denom = qMax(a, b);
        totalSil += (denom > 0.0) ? (b - a) / denom : 0.0;
    }
    return totalSil / n;
}

double KMedoids13::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

void KMedoids13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
