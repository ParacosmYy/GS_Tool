#include "GaussianMixture16.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file GaussianMixture16.cpp
 * @brief 高斯混合模型(GMM)聚类实现
 *
 * 基于EM算法拟合多高斯分量混合分布:
 * E步: 计算每个样本属于每个分量的后验概率(责任值)
 * M步: 根据责任值更新各分量的均值、协方差和混合权重
 */

/**
 * @brief 构造函数，初始化默认GMM参数
 * @param parent 父QObject对象指针
 */
GaussianMixture16::GaussianMixture16(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置高斯分量数量
 * @param count 混合模型中高斯分量的个数
 */
void GaussianMixture16::setComponentCount(int count)
{
    m_componentCount = qMax(1, count);
}

/**
 * @brief 设置EM最大迭代次数
 * @param maxIter EM算法的最大迭代轮数
 */
void GaussianMixture16::setMaxIter(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 计算多元高斯概率密度
 * @param x 数据点
 * @param mean 均值向量
 * @param variance 方差(简化为对角协方差)
 * @return 概率密度值
 */
static double gaussianPDF(const QVector<double>& x,
                           const QVector<double>& mean,
                           double variance)
{
    const int dims = x.size();
    double distSq = 0.0;
    for (int i = 0; i < dims; ++i) {
        distSq += (x[i] - mean[i]) * (x[i] - mean[i]);
    }
    const double norm = std::pow(2.0 * M_PI * variance, -dims / 2.0);
    return norm * std::exp(-distSq / (2.0 * variance));
}

/**
 * @brief 对输入数据拟合高斯混合模型
 *
 * EM算法流程:
 * 1. 初始化: 均值用K-means++或随机选取，方差为单位阵，权重均等
 * 2. E步: r_nk = pi_k * N(x_n|mu_k,sigma_k) / sum_j(pi_j * N(x_n|mu_j,sigma_j))
 * 3. M步: 更新均值、方差、权重
 * 4. 检查对数似然收敛
 *
 * @param data 输入数据矩阵
 * @return 每个样本的簇标签
 */
QVector<int> GaussianMixture16::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = data.size();
    const int dims = data[0].size();
    const int K = qMin(m_componentCount, N);

    // 初始化参数
    QVector<QVector<double>> means(K);
    QVector<double> variances(K, 1.0);
    QVector<double> weights(K, 1.0 / K);

    // 用前K个样本初始化均值
    for (int k = 0; k < K; ++k) {
        means[k] = data[k * N / K];
    }

    // EM迭代
    QVector<QVector<double>> responsibilities(N, QVector<double>(K, 0.0));

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // E步: 计算责任值
        for (int n = 0; n < N; ++n) {
            double total = 0.0;
            for (int k = 0; k < K; ++k) {
                responsibilities[n][k] = weights[k] *
                    gaussianPDF(data[n], means[k], variances[k]);
                total += responsibilities[n][k];
            }
            if (total > 1e-30) {
                for (int k = 0; k < K; ++k) {
                    responsibilities[n][k] /= total;
                }
            }
        }

        // M步: 更新参数
        for (int k = 0; k < K; ++k) {
            double Nk = 0.0;
            for (int n = 0; n < N; ++n) Nk += responsibilities[n][k];

            if (Nk < 1e-10) continue;

            // 更新权重
            weights[k] = Nk / N;

            // 更新均值
            for (int d = 0; d < dims; ++d) {
                means[k][d] = 0.0;
                for (int n = 0; n < N; ++n) {
                    means[k][d] += responsibilities[n][k] * data[n][d];
                }
                means[k][d] /= Nk;
            }

            // 更新方差(对角协方差)
            variances[k] = 0.0;
            for (int n = 0; n < N; ++n) {
                for (int d = 0; d < dims; ++d) {
                    variances[k] += responsibilities[n][k] *
                        (data[n][d] - means[k][d]) * (data[n][d] - means[k][d]);
                }
            }
            variances[k] = qMax(1e-6, variances[k] / (Nk * dims));
        }
    }

    // 分配簇标签
    QVector<int> labels(N, 0);
    for (int n = 0; n < N; ++n) {
        double maxResp = 0.0;
        for (int k = 0; k < K; ++k) {
            if (responsibilities[n][k] > maxResp) {
                maxResp = responsibilities[n][k];
                labels[n] = k;
            }
        }
    }

    m_stats.totalClustered += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClustered / N);

    emit clusteringCompleted(K);
    return labels;
}

/**
 * @brief 重置所有统计信息
 */
void GaussianMixture16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
