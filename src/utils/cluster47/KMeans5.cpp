/**
 * @file KMeans5.cpp
 * @brief K-Means聚类算法实现，支持K-Means++初始化与随机初始化
 *
 * 实现完整的K-Means聚类流程：参数配置、质心初始化（随机/K-Means++）、
 * 迭代分配与更新、收敛检测。每个公开方法均使用QElapsedTimer计时并
 * 累积统计信息到m_timeSum和m_stats。
 */

#include "utils/cluster47/KMeans5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

/**
 * @class KMeans5
 * @brief K-Means聚类器，支持可配置簇数、最大迭代次数和初始化策略
 *
 * 提供fit()训练和predict()预测两个核心接口。fit()执行完整的
 * Lloyd迭代算法直至收敛或达到最大迭代次数。支持K-Means++和
 * 随机两种质心初始化方式。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
KMeans5::KMeans5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置簇数量K
 * @param k 簇的数量，必须 >= 1
 */
void KMeans5::setComponents(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数，必须 >= 1
 */
void KMeans5::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 设置初始化方法
 * @param method 初始化方法名称，支持 "kmeans++" 和 "random"
 */
void KMeans5::setInitialization(const QString& method)
{
    if (method == "kmeans++" || method == "random") {
        m_init = method;
    }
}

/**
 * @brief 训练K-Means模型
 *
 * 根据初始化策略选择K-Means++或随机初始化质心，然后执行
 * Lloyd迭代直至收敛（质心不再变化）或达到最大迭代次数。
 * 最终计算惯性（样本到最近质心的距离平方和）。
 *
 * @param data 输入数据矩阵，每个元素为一个特征向量
 * @return 每个样本的簇标签（0 到 K-1）
 */
QVector<int> KMeans5::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 参数校验：数据为空或数据量不足 */
    if (data.isEmpty() || data.size() < m_k) {
        m_stats.totalFits++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalFits > 0)
            ? m_timeSum / m_stats.totalFits : 0.0;
        return QVector<int>();
    }

    /* 根据初始化策略初始化质心 */
    if (m_init == "kmeans++") {
        initKMeansPlusPlus(data);
    } else {
        initRandom(data);
    }

    const int n = data.size();
    const int dim = data[0].size();
    QVector<int> labels(n, 0);

    /* Lloyd迭代：交替执行分配和更新步骤 */
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;

        /* 分配步骤：将每个样本分配到最近的质心 */
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestLabel = 0;
            for (int j = 0; j < m_k; ++j) {
                double d = distance(data[i], m_centroids[j]);
                if (d < bestDist) {
                    bestDist = d;
                    bestLabel = j;
                }
            }
            if (labels[i] != bestLabel) {
                labels[i] = bestLabel;
                changed = true;
            }
        }

        /* 若无变化则已收敛 */
        if (!changed) break;

        /* 更新步骤：重新计算每个簇的质心 */
        QVector<QVector<double>> newCentroids(m_k, QVector<double>(dim, 0.0));
        QVector<int> counts(m_k, 0);

        for (int i = 0; i < n; ++i) {
            int label = labels[i];
            counts[label]++;
            for (int d = 0; d < dim; ++d) {
                newCentroids[label][d] += data[i][d];
            }
        }

        for (int j = 0; j < m_k; ++j) {
            if (counts[j] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCentroids[j][d] /= counts[j];
                }
            } else {
                /* 空簇：保留原质心 */
                newCentroids[j] = m_centroids[j];
            }
        }
        m_centroids = newCentroids;
    }

    /* 计算惯性指标（簇内距离平方和） */
    m_inertia = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = distance(data[i], m_centroids[labels[i]]);
        m_inertia += d * d;
    }

    /* 更新统计数据 */
    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFits > 0)
        ? m_timeSum / m_stats.totalFits : 0.0;

    emit fitCompleted(m_k, m_maxIter, m_inertia);
    return labels;
}

/**
 * @brief 预测新样本的簇归属
 *
 * 不修改模型状态，将每个样本分配到距离最近的质心。
 *
 * @param data 待预测的数据矩阵
 * @return 每个样本的簇标签
 */
QVector<int> KMeans5::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels;
    labels.reserve(data.size());
    for (const auto& point : data) {
        double bestDist = std::numeric_limits<double>::max();
        int bestLabel = 0;
        for (int j = 0; j < m_k; ++j) {
            double d = distance(point, m_centroids[j]);
            if (d < bestDist) {
                bestDist = d;
                bestLabel = j;
            }
        }
        labels.append(bestLabel);
    }
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void KMeans5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 随机初始化质心
 *
 * 从数据集中均匀随机选择K个样本作为初始质心。
 *
 * @param data 输入数据集
 */
void KMeans5::initRandom(const QVector<QVector<double>>& data)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, data.size() - 1);
    m_centroids.clear();
    QSet<int> chosen;
    while (m_centroids.size() < m_k) {
        int idx = dist(rng);
        if (!chosen.contains(idx)) {
            chosen.insert(idx);
            m_centroids.append(data[idx]);
        }
    }
}

/**
 * @brief K-Means++初始化质心
 *
 * 按照概率正比于距离平方的方式依次选择质心，使初始质心
 * 尽可能分散，加速收敛。
 *
 * @param data 输入数据集
 */
void KMeans5::initKMeansPlusPlus(const QVector<QVector<double>>& data)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uniDist(0, data.size() - 1);
    m_centroids.clear();

    /* 随机选择第一个质心 */
    int first = uniDist(rng);
    m_centroids.append(data[first]);

    /* 依次选择后续质心 */
    QVector<double> minDists(data.size(), std::numeric_limits<double>::max());
    for (int c = 1; c < m_k; ++c) {
        /* 更新每个样本到最近质心的距离 */
        double totalDist = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            double d = distance(data[i], m_centroids.last());
            minDists[i] = qMin(minDists[i], d * d);
            totalDist += minDists[i];
        }

        /* 按距离平方概率选择下一个质心 */
        std::uniform_real_distribution<double> probDist(0.0, totalDist);
        double threshold = probDist(rng);
        double cumulative = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            cumulative += minDists[i];
            if (cumulative >= threshold) {
                m_centroids.append(data[i]);
                break;
            }
        }
    }
}

/**
 * @brief 计算两个向量之间的欧氏距离
 * @param a 第一个向量
 * @param b 第二个向量
 * @return 欧氏距离
 */
double KMeans5::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}
