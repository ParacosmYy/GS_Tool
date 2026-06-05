/**
 * @file KMeans8.cpp
 * @brief K-Means聚类算法实现
 *
 * 实现经典K-Means++聚类算法，支持欧氏距离和曼哈顿距离度量，
 * 使用K-Means++初始化策略加速收敛。适用于中等规模数据集聚类。
 */

#include "utils/cluster71/KMeans8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
KMeans8::KMeans8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置簇数量
 * @param k 簇数量，必须>=2
 */
void KMeans8::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void KMeans8::setMaxIterations(int iter)
{
    m_maxIter = qMax(10, iter);
}

/**
 * @brief 设置距离度量类型
 * @param metric 距离度量名称: "euclidean"或"manhattan"
 */
void KMeans8::setDistanceMetric(const QString& metric)
{
    if (metric == "euclidean" || metric == "manhattan") {
        m_metric = metric;
    }
}

/**
 * @brief 执行K-Means聚类
 * @param points 输入数据点集合，每个点为D维向量
 * @return 每个点的簇标签(0~k-1)
 *
 * 算法流程:
 * 1. K-Means++初始化：按距离概率选取初始质心
 * 2. 分配步骤：将每个点分配到最近质心
 * 3. 更新步骤：重新计算各簇质心
 * 4. 重复2-3直到收敛或达到最大迭代次数
 */
QVector<int> KMeans8::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0) return QVector<int>();

    const int D = points[0].size();
    const int k = qMin(m_k, N);

    /* 阶段1: K-Means++初始化质心 */
    std::mt19937 rng(42);
    m_centroids.clear();
    m_centroids.resize(k);

    /* 随机选择第一个质心 */
    int firstIdx = rng() % N;
    m_centroids[0] = points[firstIdx];

    /* 按距离概率选取后续质心 */
    QVector<double> minDist(N, 1e18);
    for (int c = 1; c < k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < N; ++i) {
            double d = distance(points[i], m_centroids[c - 1]);
            d = d * d;
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }

        /* 按距离加权概率选择下一个质心 */
        double r = std::uniform_real_distribution<double>(0, totalDist)(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < N; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_centroids[c] = points[chosen];
    }

    /* 阶段2: 迭代优化 */
    QVector<int> labels(N, 0);
    QVector<int> clusterSizes(k, 0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;

        /* 分配步骤: 每个点分配到最近质心 */
        for (int i = 0; i < N; ++i) {
            double bestDist = 1e18;
            int bestCluster = 0;
            for (int c = 0; c < k; ++c) {
                double d = distance(points[i], m_centroids[c]);
                if (d < bestDist) {
                    bestDist = d;
                    bestCluster = c;
                }
            }
            if (labels[i] != bestCluster) {
                labels[i] = bestCluster;
                changed = true;
            }
        }

        /* 收敛检测 */
        if (!changed) break;

        /* 更新步骤: 重新计算质心 */
        m_centroids = QVector<QVector<double>>(k, QVector<double>(D, 0.0));
        clusterSizes.fill(0);
        for (int i = 0; i < N; ++i) {
            int c = labels[i];
            clusterSizes[c]++;
            for (int d = 0; d < D; ++d) {
                m_centroids[c][d] += points[i][d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (clusterSizes[c] > 0) {
                for (int d = 0; d < D; ++d) {
                    m_centroids[c][d] /= clusterSizes[c];
                }
            }
        }
    }

    /* 计算惯性(簇内平方和) */
    m_inertia = 0.0;
    for (int i = 0; i < N; ++i) {
        double d = distance(points[i], m_centroids[labels[i]]);
        m_inertia += d * d;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(k, m_inertia);
    return labels;
}

/**
 * @brief 重置统计信息
 */
void KMeans8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算两点之间的距离
 * @param a 第一个点
 * @param b 第二个点
 * @return 距离值
 *
 * 根据m_metric选择欧氏距离或曼哈顿距离。
 */
double KMeans8::distance(const QVector<double>& a, const QVector<double>& b) const
{
    const int D = qMin(a.size(), b.size());
    if (m_metric == "manhattan") {
        double dist = 0.0;
        for (int i = 0; i < D; ++i) {
            dist += qAbs(a[i] - b[i]);
        }
        return dist;
    }

    /* 默认: 欧氏距离 */
    double dist = 0.0;
    for (int i = 0; i < D; ++i) {
        double diff = a[i] - b[i];
        dist += diff * diff;
    }
    return qSqrt(dist);
}
