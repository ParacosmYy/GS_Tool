#include "GaussianMixture15.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化高斯混合模型聚类器
 * @param parent 父对象指针
 */
GaussianMixture15::GaussianMixture15(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置高斯分量数
 * @param count 分量数量(簇数)
 */
void GaussianMixture15::setComponentCount(int count)
{
    m_componentCount = qMax(1, count);
}

/**
 * @brief 设置EM最大迭代次数
 * @param maxIter 最大迭代次数
 */
void GaussianMixture15::setMaxIter(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 计算高斯概率密度
 * @param x 样本点
 * @param mean 均值
 * @param variance 方差
 * @return 概率密度值
 */
static double gaussianPdf(const QVector<double>& x,
                           const QVector<double>& mean, double variance)
{
    int dim = qMin(x.size(), mean.size());
    double diff2 = 0.0;
    for (int d = 0; d < dim; ++d) {
        double diff = x[d] - mean[d];
        diff2 += diff * diff;
    }
    double det = std::pow(qMax(1e-10, variance), dim);
    double norm = 1.0 / std::sqrt(std::pow(2.0 * M_PI, dim) * det);
    return norm * std::exp(-0.5 * diff2 / qMax(1e-10, variance));
}

/**
 * @brief 对输入数据拟合高斯混合模型(EM算法)
 *
 * EM算法迭代步骤：
 * 1. E步：计算每个样本属于每个分量的后验概率(责任度)
 *    gamma(z_nk) = pi_k * N(x_n|mu_k,sigma_k^2) / sum_j(pi_j * N(x_n|mu_j,sigma_j^2))
 * 2. M步：更新参数
 *    mu_k = sum_n(gamma_nk * x_n) / N_k
 *    sigma_k^2 = sum_n(gamma_nk * |x_n - mu_k|^2) / (N_k * dim)
 *    pi_k = N_k / N
 *
 * @param data 输入数据集
 */
void GaussianMixture15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClustered++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
        emit clusteringCompleted(0);
        return;
    }

    const int N = data.size();
    const int dim = data[0].size();
    const int K = qMin(m_componentCount, N);

    /* 初始化参数 */
    QVector<QVector<double>> means(K, QVector<double>(dim, 0.0));
    QVector<double> variances(K, 1.0);
    QVector<double> weights(K, 1.0 / K);

    /* 随机选择初始均值(K-Means++思想) */
    means[0] = data[0];
    for (int k = 1; k < K; ++k) {
        int idx = (k * N / K) % N;
        means[k] = data[idx];
    }

    /* EM迭代 */
    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* E步：计算责任度 */
        QVector<QVector<double>> gamma(N, QVector<double>(K, 0.0));

        for (int n = 0; n < N; ++n) {
            double totalProb = 0.0;
            for (int k = 0; k < K; ++k) {
                gamma[n][k] = weights[k] * gaussianPdf(data[n], means[k], variances[k]);
                totalProb += gamma[n][k];
            }
            if (totalProb > 1e-30) {
                for (int k = 0; k < K; ++k) gamma[n][k] /= totalProb;
            }
        }

        /* M步：更新参数 */
        for (int k = 0; k < K; ++k) {
            double Nk = 0.0;
            for (int n = 0; n < N; ++n) Nk += gamma[n][k];

            if (Nk < 1e-10) continue;

            /* 更新均值 */
            for (int d = 0; d < dim; ++d) {
                means[k][d] = 0.0;
                for (int n = 0; n < N; ++n) means[k][d] += gamma[n][k] * data[n][d];
                means[k][d] /= Nk;
            }

            /* 更新方差 */
            variances[k] = 0.0;
            for (int n = 0; n < N; ++n) {
                for (int d = 0; d < dim; ++d) {
                    double diff = data[n][d] - means[k][d];
                    variances[k] += gamma[n][k] * diff * diff;
                }
            }
            variances[k] /= (Nk * dim);
            variances[k] = qMax(1e-6, variances[k]);

            /* 更新权重 */
            weights[k] = Nk / N;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClustered++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
    emit clusteringCompleted(K);
}

/**
 * @brief 重置统计数据
 */
void GaussianMixture15::resetStatistics()
{
    m_stats.totalClustered = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
