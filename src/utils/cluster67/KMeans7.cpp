/**
 * @file KMeans7.cpp
 * @brief K-Means聚类算法实现
 *
 * 实现标准K-Means和Mini-Batch K-Means聚类算法，
 * 支持欧氏距离计算、聚类预测和惯性计算。
 */

#include "utils/cluster67/KMeans7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
KMeans7::KMeans7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类数量
 * @param k 聚类数，必须大于0
 */
void KMeans7::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void KMeans7::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置Mini-Batch大小
 * @param size 批大小，0表示使用全部数据（标准K-Means）
 */
void KMeans7::setMiniBatchSize(int size)
{
    m_batchSize = qMax(0, size);
}

/**
 * @brief 对数据点进行聚类
 * @param points 输入数据点集合，每个点是一个特征向量
 * @return 每个点的聚类标签
 */
QVector<int> KMeans7::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0 || m_k <= 0) return QVector<int>();

    const int D = points[0].size();

    // K-Means++初始化中心
    m_centroids.clear();
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, N - 1);
    m_centroids.append(points[uni(rng)]);

    QVector<double> minDist(N, 1e18);
    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < N; ++i) {
            double d = distance(points[i], m_centroids[c - 1]);
            minDist[i] = qMin(minDist[i], d * d);
            totalDist += minDist[i];
        }
        std::uniform_real_distribution<double> prob(0.0, totalDist);
        double r = prob(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < N; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_centroids.append(points[chosen]);
    }

    // 聚类分配
    QVector<int> labels(N, 0);

    if (m_batchSize > 0 && m_batchSize < N) {
        // Mini-Batch K-Means
        QVector<int> counts(m_k, 0);
        std::uniform_int_distribution<int> batchRng(0, N - 1);

        for (int iter = 0; iter < m_maxIter; ++iter) {
            // 采样mini-batch
            for (int b = 0; b < m_batchSize; ++b) {
                int idx = batchRng(rng);
                // 找最近中心
                int bestC = 0;
                double bestDist = 1e18;
                for (int c = 0; c < m_k; ++c) {
                    double d = distance(points[idx], m_centroids[c]);
                    if (d < bestDist) { bestDist = d; bestC = c; }
                }
                // 更新中心（流式平均）
                counts[bestC]++;
                double eta = 1.0 / counts[bestC];
                for (int d = 0; d < D; ++d) {
                    m_centroids[bestC][d] = (1.0 - eta) * m_centroids[bestC][d] + eta * points[idx][d];
                }
            }
        }
    } else {
        // 标准K-Means迭代
        for (int iter = 0; iter < m_maxIter; ++iter) {
            bool changed = false;

            // 分配步骤
            for (int i = 0; i < N; ++i) {
                int bestC = 0;
                double bestDist = 1e18;
                for (int c = 0; c < m_k; ++c) {
                    double d = distance(points[i], m_centroids[c]);
                    if (d < bestDist) { bestDist = d; bestC = c; }
                }
                if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
            }

            if (!changed) break;

            // 更新步骤：重新计算中心
            QVector<QVector<double>> newCentroids(m_k, QVector<double>(D, 0.0));
            QVector<int> counts(m_k, 0);
            for (int i = 0; i < N; ++i) {
                int c = labels[i];
                counts[c]++;
                for (int d = 0; d < D; ++d) {
                    newCentroids[c][d] += points[i][d];
                }
            }
            for (int c = 0; c < m_k; ++c) {
                if (counts[c] > 0) {
                    for (int d = 0; d < D; ++d) {
                        newCentroids[c][d] /= counts[c];
                    }
                    m_centroids[c] = newCentroids[c];
                }
            }
        }
    }

    // 计算惯性（总簇内距离平方和）
    m_inertia = 0.0;
    for (int i = 0; i < N; ++i) {
        double d = distance(points[i], m_centroids[labels[i]]);
        m_inertia += d * d;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_k, m_inertia);
    return labels;
}

/**
 * @brief 预测数据点的聚类标签
 * @param points 输入数据点集合
 * @return 每个点的聚类标签
 */
QVector<int> KMeans7::predict(const QVector<QVector<double>>& points) const
{
    QVector<int> labels;
    labels.reserve(points.size());
    for (const auto& pt : points) {
        int bestC = 0;
        double bestDist = 1e18;
        for (int c = 0; c < m_centroids.size(); ++c) {
            double d = distance(pt, m_centroids[c]);
            if (d < bestDist) { bestDist = d; bestC = c; }
        }
        labels.append(bestC);
    }
    return labels;
}

/**
 * @brief 重置统计信息
 */
void KMeans7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算欧氏距离
 * @param a 第一个向量
 * @param b 第二个向量
 * @return 欧氏距离
 */
double KMeans7::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}
