/**
 * @file KMeans4.cpp
 * @brief K-Means聚类算法实现 — 支持K-Means++初始化与随机初始化
 *
 * 实现标准K-Means聚类算法，支持:
 * - 随机初始化与K-Means++智能初始化
 * - 可配置聚类数K与最大迭代次数
 * - 计算聚类惯性(inertia)评估聚类质量
 * - 统计总拟合次数、处理点数、平均耗时
 */

#include "utils/cluster36/KMeans4.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtGlobal>

#include <cmath>
#include <random>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化KMeans4聚类器
 * @param parent QObject父对象
 */
KMeans4::KMeans4(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置聚类数K
 * @param k 聚类数量，必须 >= 1
 */
void KMeans4::setK(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数，必须 >= 1
 */
void KMeans4::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 设置初始化方法
 * @param method 初始化方法: 0=随机初始化, 1=K-Means++
 */
void KMeans4::setInitMethod(int method)
{
    m_initMethod = (method == 1) ? 1 : 0;
}

/**
 * @brief 执行K-Means聚类拟合
 *
 * 算法流程:
 * 1. 初始化质心(随机或K-Means++)
 * 2. 迭代执行: 分配每个点到最近质心 → 重新计算质心
 * 3. 收敛或达到最大迭代次数时停止
 *
 * @param data 输入数据，每个QVector<double>代表一个样本点
 * @return 每个样本点的聚类标签(0 ~ K-1)
 */
QVector<int> KMeans4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels;

    /* 空数据或K > 数据量时返回空标签 */
    if (n == 0 || m_k <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalFits++;
        m_stats.avgProcessingTimeMs =
            (m_stats.totalFits > 0) ? m_timeSum / m_stats.totalFits : 0.0;
        return labels;
    }

    const int dim = data[0].size();
    const int effectiveK = qMin(m_k, n);

    /* 初始化标签向量 */
    labels.resize(n);
    labels.fill(0);

    /* 步骤1: 初始化质心 */
    if (m_initMethod == 1) {
        /* K-Means++初始化 — 选择距离最远的点作为初始质心 */
        initCentroidsKMeansPlusPlus(data, effectiveK, dim);
    } else {
        /* 随机初始化 — 从数据中随机选择K个点 */
        initCentroidsRandom(data, effectiveK);
    }

    /* 步骤2: 迭代优化 */
    bool converged = false;
    for (int iter = 0; iter < m_maxIter && !converged; ++iter) {
        converged = true;

        /* 2a: 分配每个点到最近质心 */
        for (int i = 0; i < n; ++i) {
            int bestLabel = 0;
            double bestDist = std::numeric_limits<double>::max();

            for (int j = 0; j < effectiveK; ++j) {
                double dist = squaredDistance(data[i], m_centroids[j], dim);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestLabel = j;
                }
            }

            if (labels[i] != bestLabel) {
                converged = false;
                labels[i] = bestLabel;
            }
        }

        /* 2b: 重新计算质心 */
        QVector<QVector<double>> newCentroids(effectiveK, QVector<double>(dim, 0.0));
        QVector<int> counts(effectiveK, 0);

        for (int i = 0; i < n; ++i) {
            int lbl = labels[i];
            counts[lbl]++;
            for (int d = 0; d < dim; ++d) {
                newCentroids[lbl][d] += data[i][d];
            }
        }

        for (int j = 0; j < effectiveK; ++j) {
            if (counts[j] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCentroids[j][d] /= counts[j];
                }
            } else {
                /* 空聚类: 保留原质心 */
                newCentroids[j] = m_centroids[j];
            }
        }

        m_centroids = newCentroids;
    }

    /* 步骤3: 计算最终惯性(所有点到其质心的距离平方和) */
    m_inertia = 0.0;
    for (int i = 0; i < n; ++i) {
        m_inertia += squaredDistance(data[i], m_centroids[labels[i]], dim);
    }

    /* 更新统计信息 */
    m_stats.totalFits++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalFits > 0) ? m_timeSum / m_stats.totalFits : 0.0;

    emit fitComplete(effectiveK, m_inertia);
    return labels;
}

/**
 * @brief 获取当前质心
 * @return 质心向量，每个QVector<double>代表一个质心
 */
QVector<QVector<double>> KMeans4::centroids() const
{
    return m_centroids;
}

/**
 * @brief 获取最终惯性值
 * @return 惯性值(所有点到其质心的距离平方和)
 */
double KMeans4::inertia() const
{
    return m_inertia;
}

/**
 * @brief 重置统计信息
 */
void KMeans4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ===== 私有方法实现 ===== */

/**
 * @brief 随机初始化质心 — 从数据集中随机选择K个样本
 * @param data 数据集
 * @param k 聚类数
 */
void KMeans4::initCentroidsRandom(const QVector<QVector<double>>& data, int k)
{
    const int n = data.size();
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, n - 1);

    m_centroids.clear();
    QSet<int> chosen;

    /* 确保不重复选择 */
    while (m_centroids.size() < k) {
        int idx = dist(rng);
        if (!chosen.contains(idx)) {
            chosen.insert(idx);
            m_centroids.append(data[idx]);
        }
    }
}

/**
 * @brief K-Means++初始化 — 选择相互距离最远的点作为初始质心
 *
 * 算法:
 * 1. 随机选择第一个质心
 * 2. 计算每个点到最近已有质心的距离
 * 3. 以距离为概率选择下一个质心(距离越远越可能被选中)
 * 4. 重复直到选出K个质心
 *
 * @param data 数据集
 * @param k 聚类数
 * @param dim 数据维度
 */
void KMeans4::initCentroidsKMeansPlusPlus(const QVector<QVector<double>>& data,
                                           int k, int dim)
{
    const int n = data.size();
    if (n == 0 || k == 0) return;

    std::mt19937 rng(std::random_device{}());

    /* 随机选择第一个质心 */
    std::uniform_int_distribution<int> uni(0, n - 1);
    m_centroids.clear();
    m_centroids.append(data[uni(rng)]);

    /* 计算每个点到最近质心的最小距离 */
    QVector<double> minDist(n, std::numeric_limits<double>::max());

    for (int c = 1; c < k; ++c) {
        double totalDist = 0.0;

        /* 更新最小距离 */
        for (int i = 0; i < n; ++i) {
            double d = squaredDistance(data[i], m_centroids[c - 1], dim);
            if (d < minDist[i]) {
                minDist[i] = d;
            }
            totalDist += minDist[i];
        }

        /* 按概率选择下一个质心(距离越远概率越高) */
        if (totalDist <= 0.0) {
            /* 所有点重合，随机选一个 */
            m_centroids.append(data[uni(rng)]);
            continue;
        }

        std::uniform_real_distribution<double> prob(0.0, totalDist);
        double threshold = prob(rng);
        double cumulative = 0.0;

        for (int i = 0; i < n; ++i) {
            cumulative += minDist[i];
            if (cumulative >= threshold) {
                m_centroids.append(data[i]);
                break;
            }
        }

        /* 安全兜底 */
        if (m_centroids.size() <= c) {
            m_centroids.append(data[n - 1]);
        }
    }
}

/**
 * @brief 计算两个向量之间的欧氏距离平方
 * @param a 向量A
 * @param b 向量B
 * @param dim 维度
 * @return 欧氏距离平方
 */
double KMeans4::squaredDistance(const QVector<double>& a,
                                 const QVector<double>& b,
                                 int dim) const
{
    double sum = 0.0;
    for (int d = 0; d < dim; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return sum;
}
